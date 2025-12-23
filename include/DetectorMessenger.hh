#ifndef DETECTOR_MESSENGER_HH
#define DETECTOR_MESSENGER_HH

#include "MaterialsManager.hh"
#include "G4UImessenger.hh"
#include "globals.hh"
#include <memory>

class DetectorConstruction;
class G4UIdirectory;
class G4UIcmdWith3VectorAndUnit;
class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;

class DetectorMessenger : public G4UImessenger {
public:
    DetectorMessenger(DetectorConstruction* detector);
    ~DetectorMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValue) override;

private:
    DetectorConstruction* fDetector;

    G4UIdirectory* fDetDir;
    G4UIcmdWithAString* fAddWorldCmd;
    G4UIcmdWithAString* fAddLayerCmd;
    //G4UIcmdWithAString* fAddDetectorCmd;

    G4UIcmdWithAString* fAddElementDefCmd;
    G4UIcmdWithAString* fAddMaterialDefCmd;
};

#endif

