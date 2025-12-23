#include "RunAction.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
//#include "G4MTRunManager.hh"
#include "G4SteppingManager.hh"
#include "G4UImanager.hh"
#include "SteppingAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
//#include "G4AnalysisManager.hh"
#include "G4Threading.hh"
//#include "CsvHelper.hh"
#include "Logger.hh"

RunAction::RunAction(const std::string& reportsDir, const std::string& repBaseFilename, const bool storeStepData, const bool stepDataCsvImplIsG4, const std::vector<std::pair<G4String, char>>& g4Columns, CsvHelper& csvHelper, const std::string& stepDataTupleName): fcsvHelper(csvHelper), fReportsDir(reportsDir), fRepBaseFilename(repBaseFilename), fStoreStepData(storeStepData), fStepDataCsvImplIsG4(stepDataCsvImplIsG4) {
    if (fStoreStepData) {
        if (fStepDataCsvImplIsG4) {
            Logger::debug("G4ENGINE", "RunAction::Constructor - Initializing CSV");
            G4int tupleId = fcsvHelper.createTuple(stepDataTupleName, "Tuple for storing detailed step information", g4Columns);
        }
    }
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*) {
    Logger::debug("G4ENGINE", "Run begins (scope: worker thread)");

    if (fStoreStepData) {
        if (fStepDataCsvImplIsG4) {
            Logger::debug("G4ENGINE", "RunAction::BeginOfRunAction - Setting up Analysis Manager");
            Logger::debug("G4ENGINE", "Output data directory: " + fReportsDir);
            G4String repFileName = fReportsDir + "/" + fRepBaseFilename;
            fcsvHelper.openFile(repFileName);
        }
    }

    Logger::debug("G4ENGINE", "Run started (scope: worker thread)"); 
}

void RunAction::EndOfRunAction(const G4Run*) {
    Logger::debug("G4ENGINE", "RunAction::EndOfRunAction - Begin");
    if (fStoreStepData) {
        if (fStepDataCsvImplIsG4) {
            Logger::debug("G4ENGINE", "Closing the Analysis Manager");
            fcsvHelper.close();
        }
    }

    Logger::debug("G4ENGINE", "Run ended (scope: worker thread)");

    // SteppingAction called for gathering the summary
    //auto stepAction = dynamic_cast<SteppingAction*>(G4RunManager::GetRunManager()->GetUserSteppingAction());
    Logger::debug("G4ENGINE", "RunAction::EndOfRunAction - Creating the end of run summary (scope: worker thread)");
    SteppingAction* stepAction = dynamic_cast<SteppingAction*>(const_cast<G4UserSteppingAction*>(G4RunManager::GetRunManager()->GetUserSteppingAction()));

    if (stepAction) {
        stepAction->EndOfRunSummary();
    } else {
        Logger::error("G4ENGINE", "RunAction::EndOfRunAction - SteppingAction not found!");
    }
}
