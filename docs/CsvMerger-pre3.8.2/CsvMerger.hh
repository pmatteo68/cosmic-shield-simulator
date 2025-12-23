#ifndef CSV_MERGER_HPP
#define CSV_MERGER_HPP

#include <string>
#include <vector>

class CsvMerger {
public:
    static void merge(
        const std::string& inputDir,
        const std::string& filePattern,
        const std::string& outputDir,
        const std::string& outputFilename,
        const char inputColDelim,
        const char outputColDelim,
        const std::vector<int>& keyFields,     // input for primary key columns
        const std::vector<int>& sortingKeys,  // input for sorting fields
        bool inferHeader,
        bool keepHeader,
        int numThreads
    );

    static std::vector<int> envVarToIntVector(
        const std::string& varName,
        const std::vector<int>& defaultValue
    );

    static int envVarAsInt(
        const std::string& varName,
        int defaultValue
    );
};

#endif
