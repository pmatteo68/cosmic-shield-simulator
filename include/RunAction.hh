#ifndef RUN_ACTION_HH
#define RUN_ACTION_HH

#include "G4UserRunAction.hh"
#include "CsvHelper.hh"
//#include "StepDataWriter.hh"

class RunAction : public G4UserRunAction {
public:
    RunAction(const std::string& reportsDir, const std::string& repBaseFilename, const bool storeStepData, const bool stepDataCsvImplIsG4, const std::vector<std::pair<G4String, char>>& g4Columns, CsvHelper& csvHelper, const std::string& stepDataTupleName);
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);
private:
    CsvHelper& fcsvHelper;
    //StepDataWriter& fStepDataWr;
    mutable std::string fReportsDir;
    mutable std::string fRepBaseFilename;
    mutable bool fStepDataCsvImplIsG4;
    mutable bool fStoreStepData;
};

#endif

