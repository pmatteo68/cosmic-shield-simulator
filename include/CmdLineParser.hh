#pragma once
#include <vector>
#include <string>

class CmdLineParser {
public:
    CmdLineParser(int argc, char* argv[], const std::string& customPrefix);

    bool optionIsSpecified(const std::string& option) const;
    std::string getParamValue(const std::string& param, const std::string& defaultValue = "") const;
    bool isParamEqualTo(const std::string& param, const std::string& value, bool defaultValue = false) const;
    int getG4Argc();
    char** getG4Argv();

private:
    int fArgc;
    std::vector<std::string> fArgs;
    std::string fCustomPrefix;
    std::vector<char*> fFilteredG4Args;
    std::vector<char*> FilterG4Args(int argc, char** argv, const std::string& customPrefix);
};

