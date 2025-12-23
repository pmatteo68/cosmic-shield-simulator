#ifndef STEPPING_ACTION_HH
#define STEPPING_ACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"
#include <map>
#include <string>
#include "G4Track.hh"
//#include "G4String.hh"
#include "ParticleDataManager.hh"
#include "DetectorConstruction.hh"
#include "WRResolver.hh"
#include "CsvHelper.hh"
#include "StepDataWriter.hh"


class SteppingAction : public G4UserSteppingAction {
public:
    SteppingAction(StepDataWriter& stepDataWr, ParticleDataManager& pdm, WRResolver& wrRes, const bool saveStepData, const G4double chartZOffs, const bool stepDataCsvAllFields, DetectorConstruction& detConstr, const bool glbSummEnab, const bool thrSummEnab, const G4bool radiatON, CsvHelper& csvHelper, const bool csvIsG4Impl);
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step*);
    void EndOfRunSummary() const;

private:
    struct ParticleData {
        G4double energyIn = 0;
        G4double energyOut = 0;
        G4double energyDep = 0;
        G4double wtNormEqDose = 0;
        G4int countIn = 0;
        G4int countOut = 0;
    };

    mutable G4int fOperCount;
    mutable G4bool fWtneDoseOn;
    WRResolver& fWrRes;
    StepDataWriter& fStepDataWr;
    ParticleDataManager& fPartDataMgr;
    CsvHelper& fcsvHelper;
    mutable bool fSaveStepData;
    mutable bool fCsvIsG4Impl;
    mutable G4double fChartZOffs;
    mutable bool fStepDataCsvAllFields;
    mutable bool fGlbSummEnab;
    mutable bool fThrSummEnab;
    mutable bool fSummariesNeeded;

    //std::map<G4String, ParticleData> fDataPerType;
    mutable std::map<G4String, std::map<G4String, ParticleData>> fDataPerVolumeAndType;

    //// oldest: std::map<G4Track*, G4bool> fEnteredDetector;
    //old pre-3.7.2 std::map<G4int, G4bool> fEnteredDetector;
    //i can also do unordered map, but it requires custom hash function
    //mutable std::map<std::pair<G4int, G4int>, G4bool> fEnteredDetector;
    mutable std::map<G4String, std::map<std::pair<G4int, G4int>, G4bool>> fEnteredLayer;
    mutable std::vector<G4String> fLayerNames;
    mutable std::vector<LayerConfig> fLayers;
    //std::vector<LayerConfig> fLayers;
    DetectorConstruction& fDetConstr;

    //static bool deXbugFlow;
};

#endif
