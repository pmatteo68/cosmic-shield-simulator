#ifndef STEPDATAWRITER_HH
#define STEPDATAWRITER_HH

#include <string>
#include <vector>
#include <utility>
#include "globals.hh"  // For G4String
#include "G4Types.hh"  // For G4double, G4bool
#include "CustomCSVBuilder.hh"

class StepDataWriter {
public:
    StepDataWriter(const std::string& reportsDir,
                   const std::string& repBaseFName,
                   const bool saveStepData,
                   const G4double chartsZOffset,
                   const bool csvIsEnhanced,
                   const G4bool radiatON);
    virtual ~StepDataWriter();

    std::vector<std::pair<G4String, char>> getG4Columns();

    void open(bool withHeaders);

    void addDataField(int value);
    void addDataField(double value);
    void addDataField(char value);
    void addDataField(const std::string& value);

    void close();

private:
    std::string fReportsDir;
    std::string fRepBaseFName;
    std::string fFullPath;
    bool fSaveStepData;
    bool fIsOpen = false;
    G4double fChartsZOffset;
    bool fCsvIsEnhanced;
    G4bool fRadiatON;
    std::vector<std::string> fSdHeaders;
    std::vector<std::pair<G4String, char>> fG4Columns;
    CustomCSVBuilder* fCustCsvBuilder;
};

#endif

