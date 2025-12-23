#ifndef ACTION_INITIALIZATION_HH
#define ACTION_INITIALIZATION_HH

#include "G4VUserActionInitialization.hh"
#include <string>
#include "G4ThreeVector.hh"
#include "ParticleDataManager.hh"
#include "DetectorConstruction.hh"
#include "BeamConfigManager.hh"
#include "WRResolver.hh"
#include "StepDataWriter.hh"

class ActionInitialization : public G4VUserActionInitialization {
public:
    ActionInitialization(StepDataWriter& stepDataWr, ParticleDataManager& partDataMgr, WRResolver& wrResolv, DetectorConstruction& detectorConstr, BeamConfigManager& beamConfManager, const std::string& reportsDir, const std::string& repBaseFName, const bool saveStepData, const G4double chartsZOffset, const bool csvIsEnhanced, const bool glbSummEnab, const bool thSummEnab, const G4bool radiatON, const bool csvIsG4Impl, const std::string& stepDataTupleName);
    virtual ~ActionInitialization();

    virtual void Build() const override;
    virtual void BuildForMaster() const override;
private:
    StepDataWriter& fStepDataWr;
    ParticleDataManager& fPartDataMgr;
    WRResolver& fWrResolv;
    DetectorConstruction& fDetectorConstr;
    BeamConfigManager& fBeamConfManager;
    mutable std::string fReportsDir;
    mutable std::string fRepBaseFName;
    mutable bool fSaveStepData;
    mutable G4bool fRadiatON;
    mutable G4double fChartsZOffset;
    mutable bool fCsvIsEnhanced;
    mutable bool fGlbSummEnab;
    mutable bool fThSummEnab;
    mutable bool fCsvIsG4Impl;
    mutable std::string fStepDataTupleName;
    json LoadBeamConfig(const std::string& filename) const;
};

#endif
