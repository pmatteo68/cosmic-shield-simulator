#include "BeamConfigMessenger.hh"
#include "BeamConfigManager.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "Logger.hh"

BeamConfigMessenger::BeamConfigMessenger(BeamConfigManager* beamCfgManager)
: fBeamCfgManager(beamCfgManager)
{
    std::string baseCmd = "/css/beam";

    fDetDir = new G4UIdirectory((baseCmd + "/").c_str());
    fDetDir->SetGuidance("Commands for custom beam definition");

    fAddParticleDef = new G4UIcmdWithAString((baseCmd + "/addParticle").c_str(), this);
    fAddParticleDef->SetGuidance("Add particle definition from string");

    fAddCustomParticleDef = new G4UIcmdWithAString((baseCmd + "/defParticle").c_str(), this);
    fAddCustomParticleDef->SetGuidance("Add custom particle parameters from string");
}

BeamConfigMessenger::~BeamConfigMessenger() {
    delete fAddCustomParticleDef;
    delete fAddParticleDef;
    delete fDetDir;
}

void BeamConfigMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {

    if (command == fAddParticleDef) {
        if (fBeamCfgManager->isConfigPreloaded()) {
            Logger::warn("MACRO", "This command cannot be executed in this context (beam configuration has been already defined)");
            return;
        }

        G4double xPos, yPos, zPos, momDirX, momDirY, momDirZ, energy, flux;
        G4String particleName;
        std::istringstream iss(newValue);
        iss >> std::quoted(particleName) >> energy >> flux >> xPos >> yPos >> zPos >> momDirX >> momDirY >> momDirZ;
        G4ThreeVector pPosition = G4ThreeVector(xPos, yPos, zPos);
        G4ThreeVector pMomDir = G4ThreeVector(momDirX, momDirY, momDirZ);
        fBeamCfgManager->AddParticle(particleName, energy, flux, pPosition, pMomDir);
    }

    if (command == fAddCustomParticleDef) {
        if (fBeamCfgManager->custPartDefsLoaded()) {
            Logger::warn("MACRO", "This command cannot be executed in this context (custom particles configurations have been already defined)");
            return;
        }
        double excitEnergy;
        int Z;
        double A;
        G4String custParticleName;
        std::istringstream iss(newValue);
        iss >> std::quoted(custParticleName) >> Z >> A >> excitEnergy;
        fBeamCfgManager->AddCustomParticleDef(custParticleName, Z, A, excitEnergy);
    }
}

