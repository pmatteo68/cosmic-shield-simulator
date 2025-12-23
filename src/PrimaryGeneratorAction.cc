#include "PrimaryGeneratorAction.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"

#include "Logger.hh"

PrimaryGeneratorAction::PrimaryGeneratorAction(BeamConfigManager& beamCfgManager)
   : fBeamCfgManager(beamCfgManager) {
    fParticleGun = std::make_unique<G4ParticleGun>(1);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
    Logger::debug("G4ENGINE", "PrimaryGeneratorAction::GeneratePrimaries - Begin");
    BeamParticle p = fBeamCfgManager.SelectParticle();

    //G4cout << "[XXXXX] SelectParticle: name=" << p.name 
    //   << ", energy=" << p.energy 
    //   << ", pos=" << p.position 
    //   << ", dir=" << p.momentumDirection << G4endl;

    auto particle = G4ParticleTable::GetParticleTable()->FindParticle(p.name);
    if (!particle) {
        Logger::debug("G4ENGINE", "PrimaryGeneratorAction::GeneratePrimaries - particle NOT RESOLVED in Geant4 database: " + p.name);
        particle = fBeamCfgManager.createCustomParticleDef(p.name);
    } else {
        Logger::debug("G4ENGINE", "PrimaryGeneratorAction::GeneratePrimaries - particle RESOLVED in Geant4 database: " + p.name);
    }


    if (!particle) {
        Logger::error("G4ENGINE", "PrimaryGeneratorAction::GeneratePrimaries - Unknown particle: " + p.name);
        throw std::runtime_error("Unknown particle: " + p.name);
    }

    fParticleGun->SetParticleDefinition(particle);
    fParticleGun->SetParticleEnergy(p.energy * MeV);
    fParticleGun->SetParticlePosition(p.position);
    fParticleGun->SetParticleMomentumDirection(p.momentumDirection);
    fParticleGun->GeneratePrimaryVertex(event);
    //G4cout << "[XX1XX] Primary vertex generated for: " << particle->GetParticleName() << G4endl;

}
