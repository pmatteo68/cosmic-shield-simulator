#pragma once

#include "globals.hh"
#include "G4Types.hh"
#include <map>
#include <mutex>
#include "SurvPrimCollector.hh"
//#include "DetectorConstruction.hh"
#include "WRResolver.hh"
#include "CommonTypes.hh"

struct DerivedGeomData {
    double globNormWeight;
    double globThickness;
};

class ParticleDataManager {
private:
    struct ParticleDataRaw {
        G4double energyIn = 0;
        G4double energyOut = 0;
        G4double energyDep = 0;
        G4double wtNormEqDose = 0;
        G4int countIn = 0;
        G4int countOut = 0;
    };

    struct KPIsConfig {
        int discarded_layers;
        std::string eval_direction;
    };

    std::map<std::string, LayerEvalData> fLayersEvalDataMap;
    mutable double fGlobThickness;
    std::map<G4String, std::map<G4String, ParticleDataRaw>> fDataPerVolumeAndType;
    ////std::map<G4int, G4bool> fEnteredDetector;
    ////I can do unordered, more performant, but required custom hashing.
    //std::map<std::pair<G4int, G4int>, G4bool> fEnteredDetector;
    mutable std::mutex fMutex;
    mutable std::vector<G4String> fLayerNames;
    mutable G4String fScopeDesc;
    mutable G4bool fMapsInitialized = false;
    mutable G4bool fWtNormSvON = false;
    SurvPrimCollector& fSurvPrimColl;
    WRResolver& fWrResolver;
    //DetectorConstruction& fDetectorConstruction;
    mutable G4bool survPrimCollFinalized = false;
    mutable std::string fOutputDataDir;
    double evalEnergyEff(const double beamEnergy, const KPIsConfig& config);
    double evalProtectionEff(const double beamRadiatPot, const KPIsConfig& config);
    //double evalGlobNormWeight();
    //double evalGlobThickness();
    DerivedGeomData evalGlobGeomData();

public:
    //ParticleDataManager(const std::vector<G4String>& layerNames);
    //ParticleDataManager(DetectorConstruction& detConstruction, WRResolver& wrResol, SurvPrimCollector& survPrimColl, const G4bool radiationsON, const G4String& scopeDesc, const G4String& outputDataDir);
    ParticleDataManager(WRResolver& wrResol, SurvPrimCollector& survPrimColl, const G4bool radiationsON, const G4String& scopeDesc, const G4String& outputDataDir);
    virtual ~ParticleDataManager();

    void setGlobalGeomInfo(std::map<std::string, LayerEvalData> layersDataMap, double globalThickness);
    void initLayerDataMaps(const std::vector<G4String>& layerNames);
    //void printGlobalKPIs(double beamInitEnrg, double beamInitRadPot, double shieldThickness);
    void printGlobalKPIs(double beamInitEnrg, double beamInitRadPot);
    void printExitingParticlesSummary();
    void printGeomEvalData();
    void printPartSummary();
    void clear();
    void trackParticleIn(G4double kineticEnergy, const G4String& layer, const G4String& particleName);
    void trackParticleOut(G4double kineticEnergy, const G4String& layer, const G4String& particleName);
    void trackEnergyDeps(G4double depEnergy, G4double wtNrmEquivDose, const G4String& layer, const G4String& particleName);
    void trackSurvPrimary(const G4String& layer, G4int eventId, const G4String& particleName, G4double energy);
    ////void trackParticleInDetector(G4int trackId, G4int eventId, const G4String& particleName, G4double kineticEnergy);
    //void trackParticleInDetector(const G4String& particleName, G4double kineticEnergy);
    ////void trackParticleOutDetector(G4int trackId, G4int eventId, const G4String& particleName, G4double kineticEnergy);
    //void trackParticleOutDetector(const G4String& particleName, G4double kineticEnergy);

    const std::map<G4String, std::map<G4String, ParticleDataManager::ParticleDataRaw>>& GetData() const;

    // Allow inline access to struct from the outside via alias
    using ParticleDataRaw = ParticleDataRaw;
};

