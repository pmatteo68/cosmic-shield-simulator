#ifndef PRIMARY_GENERATOR_ACTION_HH
#define PRIMARY_GENERATOR_ACTION_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "BeamConfigManager.hh"
#include <memory>
#include <string>

class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
    PrimaryGeneratorAction(BeamConfigManager& beamCfgManager);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* event);

private:

    std::unique_ptr<G4ParticleGun> fParticleGun;
    BeamConfigManager& fBeamCfgManager;
};

#endif
