#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "G4Exception.hh"
#include "ParticleDataManager.hh"
#include "Logger.hh"
#include "CsvHelper.hh"
#include "CustomCSVBuilder.hh"

#include <fstream>
#include <sstream>

ActionInitialization::ActionInitialization(StepDataWriter& stepDataWr, ParticleDataManager& partDataMgr, WRResolver& wrResolv, DetectorConstruction& detectorConstr, BeamConfigManager& beamConfManager, const std::string& reportsDir, const std::string& repBaseFName, const bool saveStepData, const G4double chartsZOffset, const bool csvIsEnhanced, const bool glbSummEnab, const bool thSummEnab, const G4bool radiatON, const bool csvIsG4Impl, const std::string& stepDataTupleName)
 : G4VUserActionInitialization(), fStepDataTupleName(stepDataTupleName), fCsvIsG4Impl(csvIsG4Impl), fStepDataWr(stepDataWr), fPartDataMgr(partDataMgr), fWrResolv(wrResolv), fReportsDir(reportsDir), fRepBaseFName(repBaseFName), fSaveStepData(saveStepData), fChartsZOffset(chartsZOffset), fCsvIsEnhanced(csvIsEnhanced), fDetectorConstr(detectorConstr), fBeamConfManager(beamConfManager), fGlbSummEnab(glbSummEnab), fThSummEnab(thSummEnab), fRadiatON(radiatON) {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const {
    Logger::debug("G4ENGINE", "ActionInitialization::BuildForMaster - Begin");
}

void ActionInitialization::Build() const {
    Logger::debug("G4ENGINE", "ActionInitialization::Build - Begin");
    //CsvHelper csvHelp = CsvHelper();
    CsvHelper csvHelp;
    if (fSaveStepData) {
        csvHelp = CsvHelper();
    }
    Logger::debug("G4ENGINE", "Setting user action (PrimaryGeneratorAction)");
    SetUserAction(new PrimaryGeneratorAction(fBeamConfManager));

    std::vector<std::pair<G4String, char>> g4CsvColumns = fStepDataWr.getG4Columns();
    Logger::debug("G4ENGINE", "Setting user action (RunAction)");
    SetUserAction(new RunAction(fReportsDir, fRepBaseFName, fSaveStepData, fCsvIsG4Impl, g4CsvColumns, csvHelp, fStepDataTupleName));

    Logger::debug("G4ENGINE", "Setting user action (SteppingAction)");

    SetUserAction(new SteppingAction(fStepDataWr, fPartDataMgr, fWrResolv, fSaveStepData, fChartsZOffset, fCsvIsEnhanced, fDetectorConstr, fGlbSummEnab, fThSummEnab, fRadiatON, csvHelp, fCsvIsG4Impl));
}
