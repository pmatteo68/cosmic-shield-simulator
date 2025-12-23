#include "CsvMerger.hh"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <cstdlib>
#include <Logger.hh>

namespace fs = std::filesystem;

struct CsvRow {
    std::vector<std::string> fields;
    std::string fullLine;

    //bool operator<(const CsvRow& other) const {
    //    size_t fieldCount = std::min(fields.size(), other.fields.size());
    //    for (size_t i = 0; i < fieldCount; ++i) {
    //        const std::string& af = fields[i];
    //        const std::string& bf = other.fields[i];
    //        char* endA = nullptr;
    //        char* endB = nullptr;
    //        double da = std::strtod(af.c_str(), &endA);
    //        double db = std::strtod(bf.c_str(), &endB);
    //        bool aIsNum = (*endA == '\0');
    //        bool bIsNum = (*endB == '\0');
    //        if (aIsNum && bIsNum) {
    //            if (da != db) return da < db;
    //        } else {
    //            if (af != bf) return af < bf;
    //        }
    //    }
    //    return false;
    //}
};

bool matchPattern(const std::string& filename, const std::string& pattern) {
    if (pattern.empty()) return true;

    std::string regexPattern = std::regex_replace(pattern, std::regex(R"([\.\+\^\$\(\)\[\]\{\}])"), R"(\$&)");
    regexPattern = std::regex_replace(regexPattern, std::regex(R"(\*)"), ".*");
    std::regex re(regexPattern);
    return std::regex_match(filename, re);
}

void logWarning(const std::string& message) {
    Logger::warn("CSVMERGER", message);
    //std::ceXrr << "[WARNING] " << message << '\n';
}

void processFileChunk(const std::vector<fs::path>& files, size_t startIdx, size_t endIdx,
                      std::vector<CsvRow>& output, const char colDelimiter, bool skipHeader, std::mutex& outputMutex,
                      const std::vector<int>& keyFields, const std::vector<int>& sortingKeys,
                      std::unordered_map<std::string, std::vector<std::string>>& seenKeys) {
    std::vector<CsvRow> localRows;

    for (size_t i = startIdx; i < endIdx; ++i) {
        std::ifstream infile(files[i]);
        if (!infile.is_open()) {
            logWarning("Failed to open: " + files[i].string());
            continue;
        }

        std::string line;
        bool headerSkipped = false;

        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue;

            if (skipHeader && !headerSkipped) {
                headerSkipped = true;
                continue;
            }

            std::istringstream ss(line);
            std::string field;
            std::vector<std::string> fields;

            while (std::getline(ss, field, colDelimiter)) {
                fields.push_back(field);
            }

            // Create the key based on specified columns
            std::string key;
            for (int idx : keyFields) {
                if (idx < 0 || idx >= fields.size()) continue;
                key += fields[idx] + ",";
            }

            if (!key.empty()) key.pop_back(); // Remove trailing comma

            // Check for duplicate key and log warning
            if (seenKeys.find(key) != seenKeys.end()) {
                logWarning("Duplicate found: " + key + " in file: " + files[i].string());
            }

            seenKeys[key] = fields;
            localRows.push_back({fields, line});
        }
    }

    // Lock and move local rows to shared vector
    std::lock_guard<std::mutex> lock(outputMutex);
    output.insert(output.end(), localRows.begin(), localRows.end());
}

void CsvMerger::merge(
    const std::string& inputDir,
    const std::string& filePattern,
    const std::string& outputDir,
    const std::string& outputFilename,
    const char inputColDelim,
    const char outputColDelim,
    const std::vector<int>& keyFields,
    const std::vector<int>& sortingKeys,
    bool inferHeader,
    bool keepHeader,
    int numThreads
) {

    Logger::info("CSVMERGER", "CSV Merging process begin (Threads: " + std::to_string(numThreads) + ").");
    Logger::info("CSVMERGER", "Input: " + inputDir + "/" + filePattern + ".");
    Logger::info("CSVMERGER", "Output: " + outputDir + "/" + outputFilename + ".");
    //std::cxout << "[INFO][CSVMERGER] Input: " << inputDir << " (" << filePattern << ").\n";
    //std::xcxout << "[INFO][CSVMERGER] Output: " << outputDir << "/" << outputFilename << ".\n";
    std::vector<fs::path> matchedFiles;
    std::vector<std::string> columnNames;

    int ifiles = 0;
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file() && matchPattern(entry.path().filename().string(), filePattern)) {
            Logger::debug("CSVMERGER", "File found: " + entry.path().filename().string() + ".");
            //std::cXout << "[DEBUG][CSVMERGER] File found: " << entry.path().filename().string() << ".\n";
            matchedFiles.push_back(entry.path());
            ifiles++;
        }
    }

    if (matchedFiles.empty()) {
        logWarning("No matching files found in: " + inputDir);
        return;
    }

    Logger::info("CSVMERGER", "Files found: " + std::to_string(ifiles) + ".");
    //bool inferXXHeader = true;
    if (inferHeader) {
        std::ifstream infile(matchedFiles[0]);
        if (infile.is_open()) {
            std::string line;
            ////std::regex columnRegex(R"#(^#\s*\w+\s+(\w+))#");
            std::regex columnRegex(R"#(^#\s*\w+\s+\w+\s+(\w+))#");
            //std::regex columnRegex(R"#(^#\s*\w+\s+(\w+)\s+(\w+))#");
            while (std::getline(infile, line)) {
                if (line.rfind("#column", 0) == 0) {
                    std::smatch match;
                    if (std::regex_search(line, match, columnRegex) && match.size() > 1) {
                        //std::string colXType = match[1];
                        std::string colName = match[1];
                        //std::coXut << "[DEBUG][CSVMERGER] Parsed column metadata: " << colName << " (" << colType << ")" << '\n';
                        //std::cXout << "[DEBUG][CSVMERGER] Parsed column metadata: " << colName << '\n';
                        Logger::debug("CSVMERGER", "Parsed column metadata: " + colName + ".");
                        columnNames.push_back(colName);
                    }
                }
            }
        }
    }

    std::vector<CsvRow> allRows;
    std::mutex outputMutex;
    std::unordered_map<std::string, std::vector<std::string>> seenKeys;

    std::string headerLine;
    if (keepHeader) {
        std::ifstream headerFile(matchedFiles[0]);
        while (std::getline(headerFile, headerLine)) {
            if (!headerLine.empty() && headerLine[0] != '#') break;
        }
    }

    size_t totalFiles = matchedFiles.size();
    numThreads = std::max(1, std::min(numThreads, static_cast<int>(totalFiles)));
    std::vector<std::thread> threads;
    size_t chunkSize = (totalFiles + numThreads - 1) / numThreads;

    Logger::info("CSVMERGER", "Processing " + std::to_string(totalFiles) + " files in " + std::to_string(numThreads) + " threads...");
    //std::coXut << "[DEBUG][CSVMERGER] Processing " << totalFiles << " files in " << numThreads << " threads...\n";

    for (int t = 0; t < numThreads; ++t) {
        size_t startIdx = t * chunkSize;
        size_t endIdx = std::min(startIdx + chunkSize, totalFiles);
        threads.emplace_back(processFileChunk, std::ref(matchedFiles), startIdx, endIdx,
                             std::ref(allRows), std::ref(inputColDelim), /*skipHeader=*/keepHeader && t > 0,
                             std::ref(outputMutex), std::ref(keyFields), std::ref(sortingKeys),
                             std::ref(seenKeys));
    }

    for (auto& t : threads) t.join();

    // Sorting based on sortingKeys (numeric if both fields are numeric)
    //std::sort(allRows.begin(), allRows.end(), [&sortingKeys](const CsvRow& a, const CsvRow& b) {
    //    for (size_t i = 0; i < sortingKeys.size(); ++i) {
    //        int idx = sortingKeys[i];
    //        if (a.fields[idx] != b.fields[idx]) {
    //            return a.fields[idx] < b.fields[idx];
    //        }
    //    }
    //    return false;
    //});
    std::sort(allRows.begin(), allRows.end(), [&sortingKeys](const CsvRow& a, const CsvRow& b) {
        for (size_t i = 0; i < sortingKeys.size(); ++i) {
            int idx = sortingKeys[i];
            const std::string& af = a.fields[idx];
            const std::string& bf = b.fields[idx];

            char* endA = nullptr;
            char* endB = nullptr;
            double da = std::strtod(af.c_str(), &endA);
            double db = std::strtod(bf.c_str(), &endB);

            bool aIsNum = (*endA == '\0');
            bool bIsNum = (*endB == '\0');

            if (aIsNum && bIsNum) {
                if (da != db) return da < db;
            } else {
                if (af != bf) return af < bf;
            }
        }
        return false;
    });


    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    std::ofstream outfile(fs::path(outputDir) / outputFilename);
    if (!outfile.is_open()) {
        logWarning("Cannot write to file: " + (fs::path(outputDir) / outputFilename).string());
        return;
    }


    if (inferHeader) {
      if (!columnNames.empty()) {
        for (size_t i = 0; i < columnNames.size(); ++i) {
            if (i > 0) outfile << outputColDelim;
            outfile << columnNames[i];
        }
        outfile << '\n';
      }
    }

    if (keepHeader && !headerLine.empty()) {
        outfile << headerLine << '\n';
    }

    size_t processed = 0;
    for (const auto& row : allRows) {
        if (inputColDelim == outputColDelim) {
            outfile << row.fullLine << '\n';
        } else {
            for (size_t i = 0; i < row.fields.size(); ++i) {
                if (i > 0) outfile << outputColDelim;
                outfile << row.fields[i];
            }
            outfile << '\n';
        }
        processed++;
        if (processed % 1000 == 0) {
            //std::cxout << "[DEBUG][CSVMERGER] Processed " << processed << " rows...\n";
            Logger::info("CSVMERGER", "Processed " + std::to_string(processed) + " rows...");
        }
    }

    Logger::info("CSVMERGER", "CSV Merging process complete. Total rows written: " + std::to_string(processed) + ".");
    //std::cxxout << "[INFO][CSVMERGER] Processing complete. Total rows written: " << processed << '\n';
}

std::vector<int> CsvMerger::envVarToIntVector(
        const std::string& varName,
        const std::vector<int>& defaultValue) {
    const char* envVal = std::getenv(varName.c_str());
    if (!envVal) {
        Logger::warn("CSVMERGER", "Environment variable " + varName + " not set. Using default.");
        //std::cXerr << "Environment variable " << varName << " not set. Using default.\n";
        return defaultValue;
    }

    std::vector<int> result;
    std::stringstream ss(envVal);
    std::string token;

    while (std::getline(ss, token, ',')) {
        try {
            result.push_back(std::stoi(token));
        } catch (const std::exception& e) {
            Logger::error("CSVMERGER", "Invalid entry in " + varName + ": " + token + ". Skipping.");
            //std::cXerr << "Invalid entry in " << varName << ": " << token << " — skipping\n";
        }
    }

    return result.empty() ? defaultValue : result;
}

int CsvMerger::envVarAsInt(
        const std::string& varName,
        int defaultValue
    )
{
    const char* envVal = std::getenv(varName.c_str());
    if (!envVal) {
        return defaultValue;
    }

    try {
        return std::stoi(envVal);
    } catch (const std::exception& e) {
        Logger::warn("CSVMERGER", "Invalid int in env var " + varName + ": " + envVal + ". Using default.");
        //std::ceXrr << "Invalid int in env var " << varName << ": " << envVal << " — using default.\n";
        return defaultValue;
    }
}

