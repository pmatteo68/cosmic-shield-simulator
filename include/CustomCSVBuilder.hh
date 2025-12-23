#ifndef CUSTOM_CSV_BUILDER_HH
#define CUSTOM_CSV_BUILDER_HH

#include <fstream>
#include <mutex>
#include <string>
#include <vector>

class CustomCSVBuilder {
public:
    CustomCSVBuilder(const std::string& separator, const std::vector<std::string>& headers, const std::string& filePath);
    ~CustomCSVBuilder();

    void open(const bool withHeader);
    void addDataField(int value);
    void addDataField(double value);
    void addDataField(char value);
    void addDataField(const std::string& value);
    void close();

private:
    void writeDoubleField(const double value);
    void writeField(const std::string& value);
    void addNL();

    std::string fSeparator;
    int fNumCols = 0;
    int fColCount = 0;
    std::vector<std::string> fHeaders;
    std::string fFilePath;
    std::ofstream* fFile;
    std::mutex fMutex;
};

#endif // CUSTOM_CSV_BUILDER_HH

