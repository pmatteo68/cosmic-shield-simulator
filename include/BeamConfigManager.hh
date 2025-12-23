#ifndef BEAM_CFGMGR_HH
#define BEAM_CFGMGR_HH

#include "G4String.hh"
#include "G4ThreeVector.hh"
#include <vector>
#include "BeamConfigMessenger.hh"
#include "CustomIonLoader.hh"
#include "nlohmann/json.hpp"
#include "G4ParticleDefinition.hh"
#include "WRResolver.hh"

using json = nlohmann::json;

struct BeamParticle {
    G4String name;
    G4double energy;                 // MeV
    G4double flux;                   // particles / m^2 / s
    G4ThreeVector position;          // mm
    G4ThreeVector momentumDirection; // unit vector
};

class BeamConfigManager {
public:
    BeamConfigManager(CustomIonLoader& custIonLoader, WRResolver& wrResol, const G4String& beamConfFile, const G4String& outputDataDir);
    virtual ~BeamConfigManager();

    void AddCustomParticleDef(const std::string& myName, int Z, double A, double excitationEnergy);
    bool custPartDefsLoaded();

    void AddParticle(const G4String& name,
                     G4double energy,
                     G4double flux,
                     const G4ThreeVector& position,
                     const G4ThreeVector& momDir);
                     //const G4ThreeVector& defMomDir);

    G4ParticleDefinition* createCustomParticleDef(const std::string& myParticleName);
    G4bool isConfigPreloaded() const;
    BeamParticle SelectParticle();
    void ReportParticleUsage() const;
    double GetBeamInitEnergy() const;
    double GetBeamInitRadiationPotential() const;
    void LoadConfigFromFile();

private:
    G4bool fIsConfigLoadedFromFile = false;
    G4int fFallbackSelectionsCount = 0;
    G4int fSelectionsCount = 0;
    std::mutex fMutex;
    G4double fTotalFlux = 0.0;
    G4String fBeamConfFile;
    mutable G4String fOutputDataDir;
    mutable json fBeamConfig;
    mutable G4ThreeVector fDefPosition = G4ThreeVector(0.0, 0.0, -1.0);
    mutable G4ThreeVector fDefMomentumDir = G4ThreeVector(0.0, 0.0, 1.0);
    CustomIonLoader& fCustIonLoader;
    WRResolver& fWrResolver;

    std::unique_ptr<BeamConfigMessenger> fMessenger;
    std::vector<BeamParticle> fBeamComposition;
    std::vector<G4int> fParticleUseCounts;
    json GetJsonConfigHandle() const;
};

#endif // BEAM_CFGMGR_HH
