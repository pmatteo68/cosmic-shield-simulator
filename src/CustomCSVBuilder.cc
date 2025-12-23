#include "CustomCSVBuilder.hh"
#include "Logger.hh"
#include <stdexcept>
#include <iomanip>
#include <limits>

CustomCSVBuilder::CustomCSVBuilder(const std::string& separator, const std::vector<std::string>& headers, const std::string& filePath)
    : fSeparator(separator), fHeaders(headers), fFilePath(filePath), fFile(nullptr) {
    fNumCols = fHeaders.size();
    Logger::debug("CUSTOMCSV", "[" + fFilePath + "] Build - begin, cols: " + std::to_string(fNumCols));
}

CustomCSVBuilder::~CustomCSVBuilder() {
    close();
}

void CustomCSVBuilder::open(const bool withHeader) {
    std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("CUSTOMCSV", "[" + fFilePath + "] Opening file"); 
    if (!fFile) {
        fFile = new std::ofstream(fFilePath, std::ios::out);
        if (!fFile->is_open()) {
            delete fFile;
            fFile = nullptr;
            throw std::runtime_error("Failed to open CSV file: " + fFilePath);
        }
        Logger::debug("CUSTOMCSV", "[" + fFilePath + "] File opened");
        if (withHeader) {
            for (size_t i = 0; i < fHeaders.size(); ++i) {
                if (i != 0) (*fFile) << fSeparator;
                Logger::debug("CUSTOMCSV", "[" + fFilePath + "] Adding header(" + std::to_string(i) + "): " + fHeaders[i]);
                (*fFile) << fHeaders[i];
            }
            addNL();
        }
    } else {
        Logger::warn("CUSTOMCSV", "[" + fFilePath + "] File resulted already open");
    }
}

void CustomCSVBuilder::addDataField(int value) {
    std::lock_guard<std::mutex> lock(fMutex);
    writeField(std::to_string(value));
}

void CustomCSVBuilder::addDataField(double value) {
    std::lock_guard<std::mutex> lock(fMutex);
    //writeField(std::to_string(value));
    writeDoubleField(value);
}

void CustomCSVBuilder::addDataField(char value) {
    std::lock_guard<std::mutex> lock(fMutex);
    writeField(std::string(1, value));
}

void CustomCSVBuilder::addDataField(const std::string& value) {
    std::lock_guard<std::mutex> lock(fMutex);
    writeField(value);
}

void CustomCSVBuilder::addNL() {
    //std::lock_guard<std::mutex> lock(fMutex);
    if (fFile) {
        (*fFile) << "\r\n";
    }
    fColCount = 0;
}

void CustomCSVBuilder::close() {
    std::lock_guard<std::mutex> lock(fMutex);
    if (fFile) {
        fFile->close();
        delete fFile;
        fFile = nullptr;
    }
}

void CustomCSVBuilder::writeField(const std::string& value) {
    if (!fFile) return;
    if (fColCount > 0) {
        (*fFile) << fSeparator;
        //fFile << fSeparator;
    }
    (*fFile) << value;
    fColCount++;
    if (fColCount >= fNumCols) {
        addNL();
    }
}

void CustomCSVBuilder::writeDoubleField(const double value) {
    if (!fFile) return;
    if (fColCount > 0) {
        (*fFile) << fSeparator;
    }
    //(*fFile) << value;
    (*fFile) << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    fColCount++;
    if (fColCount >= fNumCols) {
        addNL();
    }
}

