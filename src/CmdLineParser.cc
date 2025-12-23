#include "CmdLineParser.hh"
#include <algorithm>
#include "Logger.hh"

CmdLineParser::CmdLineParser(int argc, char* argv[], const std::string& customPrefix): fArgc(argc), fCustomPrefix(customPrefix) {
    // Store command line arguments in a vector for easy access
    for (int i = 0; i < argc; ++i) {
        fArgs.push_back(argv[i]);
    }
    fFilteredG4Args = FilterG4Args(argc, argv, fCustomPrefix);
}

int CmdLineParser::getG4Argc() {
    int g4argc = static_cast<int>(fFilteredG4Args.size());
    return (g4argc);
}

char** CmdLineParser::getG4Argv() {
    char** g4argv = fFilteredG4Args.data();
    int g4argc = getG4Argc();
    for (int i = 0; i < g4argc; ++i) {
        Logger::debug("CMD", "Filtered argument g4argv[" + std::to_string(i) + "]: " + g4argv[i]);
    }
    return (g4argv);
}

bool CmdLineParser::optionIsSpecified(const std::string& option) const {
    return std::find(fArgs.begin(), fArgs.end(), "-" + option) != fArgs.end();
}

std::string CmdLineParser::getParamValue(const std::string& param, const std::string& defaultValue) const {
    auto it = std::find(fArgs.begin(), fArgs.end(), "--" + param);
    if (it != fArgs.end() && ++it != fArgs.end()) {
        return *it;  // Return the token right after the parameter
    }
    return defaultValue;  // Return default value if parameter is not found
}

bool CmdLineParser::isParamEqualTo(const std::string& param, const std::string& value, bool defaultValue) const {
    std::string paramValue = getParamValue(param);
    if (paramValue.empty()) {
        return defaultValue;  // Return default value if parameter is not found
    }
    return paramValue == value;  // Compare the parameter value with the string
}

std::vector<char*> CmdLineParser::FilterG4Args(int argc, char** argv, const std::string& customPrefix) {
    std::vector<char*> result;
    result.push_back(argv[0]); // Always keep program name

    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (!(arg.rfind("-" + customPrefix, 0) == 0 || arg.rfind("--" + customPrefix, 0) == 0)) {
            Logger::debug("CMD", std::string("G4 Filtered arg: ") + arg);
            result.push_back(argv[i]);
        }
    }
    return result;
}

