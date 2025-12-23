#ifndef BEAMCFG_MESSENGER_HH
#define BEAMCFG_MESSENGER_HH

//#include "MaterialsManager.hh"
#include "G4UImessenger.hh"
#include "globals.hh"
#include <memory>

class BeamConfigManager;
class G4UIdirectory;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

class BeamConfigMessenger : public G4UImessenger {
public:
    BeamConfigMessenger(BeamConfigManager* beamCfgManager);
    ~BeamConfigMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    BeamConfigManager* fBeamCfgManager;

    G4UIdirectory* fDetDir;
    G4UIcmdWithAString* fAddParticleDef;
    G4UIcmdWithAString* fAddCustomParticleDef;
};

#endif

