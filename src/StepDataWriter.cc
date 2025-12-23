#include "StepDataWriter.hh"
#include "Logger.hh"

//static vars
std::string sdCsvSeparator = std::getenv("CSS_STEPDATA_CSV_SEP") ? std::getenv("CSS_STEPDATA_CSV_SEP") : ";";

StepDataWriter::StepDataWriter(const std::string& reportsDir,
                               const std::string& repBaseFName,
                               const bool saveStepData,
                               const G4double chartsZOffset,
                               const bool csvIsEnhanced,
                               const G4bool radiatON)
: fReportsDir(reportsDir),
  fRepBaseFName(repBaseFName),
  fSaveStepData(saveStepData),
  fChartsZOffset(chartsZOffset),
  fCsvIsEnhanced(csvIsEnhanced),
  fRadiatON(radiatON)
{
    Logger::debug("STEPDWR", "Constructor - begin");
    fG4Columns = {
        {"EventId", 'D'},
        {"TrackId", 'D'},
        {"StepNbr", 'D'},
        {"ThreadId", 'D'},
        {"zPos", 'D'},
        {"StepLen", 'D'},
        {"Particle", 'S'},
        {"VolPre", 'S'},
        {"VolPost", 'S'},
        {"kineticEnergy", 'D'},
        {"residualEnergy", 'D'}
    };
    if (fCsvIsEnhanced) {
        Logger::debug("STEPDWR", "Adding enhancement columns");
        std::vector<std::pair<G4String, char>> g4ColumnsToAdd = {
            {"xPos", 'D'},
            {"yPos", 'D'},
            {"momDirX", 'D'},
            {"momDirY", 'D'},
            {"momDirZ", 'D'},
            {"depEnergy", 'D'},
            {"TrackParentId", 'D'},
            {"mass", 'D'},
            {"Process", 'S'}
        };
        fG4Columns.insert(fG4Columns.end(), g4ColumnsToAdd.begin(), g4ColumnsToAdd.end());
        if (fRadiatON) {
            Logger::debug("STEPDWR", "Adding radiation column");
            fG4Columns.emplace_back("WTNEDose", 'D');
        }
    }

    //std::vector<std::string> fSdHeaders;
    for (const auto& col : fG4Columns) {
        fSdHeaders.push_back(col.first);
    }

}

StepDataWriter::~StepDataWriter() {}

std::vector<std::pair<G4String, char>> StepDataWriter::getG4Columns() {
    return fG4Columns;
}

void StepDataWriter::open(bool withHeaders) {
    if (fSaveStepData) {
        //G4String fullPathCsv = fReportsDir + "/" + fRepBaseFName;
        fFullPath = fReportsDir + "/" + fRepBaseFName;
        Logger::debug("STEPDWR", "Opening: " + fFullPath);
        fCustCsvBuilder = new CustomCSVBuilder(sdCsvSeparator, fSdHeaders, fFullPath);
        fCustCsvBuilder->open(withHeaders);
        fIsOpen = true;
    }
}

void StepDataWriter::addDataField(int value) {
    fCustCsvBuilder->addDataField(value);
}

void StepDataWriter::addDataField(double value) {
    fCustCsvBuilder->addDataField(value);
}

void StepDataWriter::addDataField(char value) {
    fCustCsvBuilder->addDataField(value);
}

void StepDataWriter::addDataField(const std::string& value) {
    fCustCsvBuilder->addDataField(value);
}

void StepDataWriter::close() {
    if (fSaveStepData) {
        if (fIsOpen) {
            Logger::debug("STEPDWR", "Closing");
            fCustCsvBuilder->close();
            Logger::info("STEPDWR", "Step data CSV saved to: " + fFullPath);
        }
    }
}

