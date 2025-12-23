#ifndef DETECTOR_CONSTRUCTION_HH
#define DETECTOR_CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include <memory>
#include <vector>
#include "G4ThreeVector.hh"
#include "DetectorMessenger.hh"
#include "CustomMaterialLoader.hh"
#include "ParticleDataManager.hh"
#include "CommonTypes.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class MaterialsManager;

struct LayerConfig;
struct WorldConfig;
struct DetectorConfig;

//struct LayerEvalData {
//    double density;
//    double weightPerSurfUnit;
//    //double lThickness;
//};

//class DetectorMessenger; // forward decl

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
    //DetectorConstruction();
    DetectorConstruction(const std::string& geomConfFile, CustomMaterialLoader& custMatLoader);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();

    G4LogicalVolume* GetSensitiveVolume() const { return fSensitiveVolume; }

    const std::vector<LayerConfig>& GetLayerConfigs() const;
    const std::vector<G4String>& getLayerNames() const;

    void AddWorld(const WorldConfig& worldConf);
    void AddLayer(const LayerConfig& layer);
    //void AddDetector(const DetectorConfig& detectorCfg);
    G4bool isGeomPreloaded();
    CustomMaterialLoader& getCustMatLoader();

    ////std::map<std::string, LayerEvalData>& getLayersEvalDataMap();
    //double GetShieldThickness();
    //std::map<std::string, LayerEvalData> getLayersEvalDataMapCopy();
    void setParticleDataManagerRef(ParticleDataManager& pdmRef);

private:
    //ParticleDataManager& fParticleDataManagerRef;
    ParticleDataManager* fParticleDataManagerPtr = nullptr;

    mutable std::map<std::string, LayerEvalData> fLayersEvalDataMap;
    std::unique_ptr<DetectorMessenger> fMessenger;
    mutable G4int fKCurrCopyNumber = 0;
    mutable G4int fNumLayersAdded = 0;
    const G4bool fKIsManyPlacement = false;
    mutable G4bool fGeomLoaded = false;
    mutable G4double fCommonXPos;
    mutable G4double fCommonYPos;
    mutable G4double fWorldZPos;
    mutable G4double fCommonSzX;
    mutable G4double fCommonSzY;
    mutable G4double fCurrentZ;
    G4VPhysicalVolume* fPhysWorld;
    mutable double fShieldThickness = 0.0;

    std::unique_ptr<MaterialsManager> fMaterialsManager;
    G4LogicalVolume* fSensitiveVolume = nullptr;
    G4LogicalVolume* fLogicWorld = nullptr;

    //G4double fWorldSizeXY = 1.0 * m;
    //G4double fWorldSizeZ  = 2.0 * m;
    mutable G4ThreeVector fWorldPosition;
    std::string fGeomConfFile;
    //std::string worldMaterial;
    std::vector<G4String> fLayerNames;
    mutable std::vector<LayerConfig> fLayers;
    CustomMaterialLoader& fCustMatLoader;

    void clear();
};

#endif
