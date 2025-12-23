#include "DetectorConstruction.hh"
#include "MaterialsManager.hh"

#include "G4Material.hh"
//#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4UserLimits.hh"
#include "Logger.hh"
#include "Utils.hh"
//#include "G4NistManager.hh"
//#include "DetectorMessenger.hh"

DetectorConstruction::DetectorConstruction(const std::string& geomConfFile, CustomMaterialLoader& custMatLoader)
  : G4VUserDetectorConstruction(), fGeomConfFile(geomConfFile), fCustMatLoader(custMatLoader) {
    Logger::debug("GEOMETRY", "DetectorConstruction::DetectorConstruction - Begin");
    fMessenger = std::make_unique<DetectorMessenger>(this);
    fMaterialsManager = std::make_unique<MaterialsManager>(custMatLoader);
    //std::string geomConfigPath = fConfXigDir + "/geomXetry.json";
    this->clear();
    //fLayerxNames.cXlear(); // Clear in case this is called multiple times
    //fLayerxs.cleXar();
    G4int numLayers = 0;
    try {
        fMaterialsManager->LoadFromJson(geomConfFile);
        fLayers = fMaterialsManager->GetLayerConfigs();
        numLayers = fLayers.size();
        for (size_t i = 0; i < numLayers; ++i) {
            const auto& layer = fLayers[i];
            const auto& layerName = layer.name;
            Logger::debug("GEOMETRY", "DetectorConstruction: populating fLayerNames with: " + layerName);
            fLayerNames.push_back(layerName);
        }
        if (numLayers > 0) {
            fGeomLoaded = true;
        }
    } catch (const std::exception& ex) {
        Logger::warn("GEOMETRY", std::string("DetectorConstruction: geometry could not be loaded from configuration file: ") + ex.what());
        this->clear();
        //fLaXyers.clXear();
        //fLayXerNames.clxear();
        fGeomLoaded = false;
    }
    Logger::debug("GEOMETRY", "fGeomLoaded set to " + std::to_string(fGeomLoaded) + " after loading from json (num. layers: " + std::to_string(numLayers) + ")");
}

DetectorConstruction::~DetectorConstruction() {
}

void DetectorConstruction::setParticleDataManagerRef(ParticleDataManager& pdmRef) {
    fParticleDataManagerPtr = &pdmRef;
}

G4bool DetectorConstruction::isGeomPreloaded()
{
    return (fGeomLoaded);
}

//double DetectorConstruction::GetShieldThickness()
//{
//    return (fShieldThickness);
//}

CustomMaterialLoader& DetectorConstruction::getCustMatLoader() {
    return fCustMatLoader;
}

//std::map<std::string, LayerEvalData>& DetectorConstruction::getLayersEvalDataMap() {
//    //Logger::info("DEBUG4 thick  ", " PbLayer " + std::to_string(fLayersEvalDataMap["PbLayer"].lThickness));
//    //Logger::info("DEBUG4 weight ", " PbLayer " + std::to_string(fLayersEvalDataMap["PbLayer"].weightPerSurfUnit));
//    return fLayersEvalDataMap;
//}

//std::map<std::string, LayerEvalData> DetectorConstruction::getLayersEvalDataMapCopy() {
//    return fLayersEvalDataMap;
//}


void DetectorConstruction::clear() {
    fLayers.clear();
    fLayerNames.clear();
    fLayersEvalDataMap.clear();
    //fLayeXrIndexes.clXear();
    fKCurrCopyNumber = 0;
    fNumLayersAdded = 0;
    Logger::debug("GEOMETRY", "Geometry CLEARED");
}

void DetectorConstruction::AddWorld(const WorldConfig& worldConf) {
    // (Copy world construction code from Construct(), unchanged except reuse worldConf parameter)
    // Assign fLogicWorld and physWorld to class members if needed later
    //auto nxist = G4NistManager::Instance();
    //WorldConfig worldConf = fMaterialsManager->GetWorldConfig();
    std::string worldMaterial = worldConf.materialName;
    //G4Material* worldMat = nixst->FindOrBuildMaterial(worldMaterial);
    G4Material* worldMat = fMaterialsManager->GetOrBuildMaterial(worldMaterial);

    fKCurrCopyNumber = 0;
    //fKIsManyPlacement = false;

    // World
    const auto worldName = "World";
    //assessing position
    fWorldPosition = G4ThreeVector(0.0, 0.0, 0.0) * m;
    if (worldConf.position.has_value()) {
        const auto& wpos = worldConf.position.value();
        fWorldPosition = G4ThreeVector(wpos.x(), wpos.y(), wpos.z()) * m;
    }
    const G4double worldXPos = fWorldPosition.x();
    const G4double worldYPos = fWorldPosition.y();
    fCommonXPos = worldXPos;
    fCommonYPos = worldYPos;
    fWorldZPos = fWorldPosition.z();

    G4double worldSzX = worldConf.xSize * m;
    G4double worldSzY = worldConf.ySize * m;
    G4double worldSzZ = worldConf.zSize * m;

    fCommonSzX = worldSzX;
    fCommonSzY = worldSzY;
    auto solidWorld = new G4Box(worldName, worldSzX / 2, worldSzY / 2, worldSzZ / 2);
    //auto solidWorld = new G4Box(worldName, fWorldSizeXY / 2, fWorldSizeXY / 2, fWorldSizeZ / 2);
    fLogicWorld = new G4LogicalVolume(solidWorld, worldMat, worldName);
    std::string worldStepLimDesc;
    worldStepLimDesc = "default";
    if (worldConf.maxStep.has_value()) {
        G4double worldStepLimit = worldConf.maxStep.value() * mm;
        G4UserLimits* wuserLimits = new G4UserLimits(worldStepLimit);
        fLogicWorld->SetUserLimits(wuserLimits);
        worldStepLimDesc = std::to_string(worldConf.maxStep.value()) + " mm";
    }
    fPhysWorld = new G4PVPlacement(nullptr, G4ThreeVector(worldXPos, worldYPos, fWorldZPos), fLogicWorld, worldName, nullptr, fKIsManyPlacement, fKCurrCopyNumber);
    Logger::debug("GEOMETRY", "First volume (" + std::string(worldName) + ", copy nbr: " + std::to_string(fKCurrCopyNumber) + ") placed at (" + std::to_string(worldXPos) + ", " + std::to_string(worldYPos) + ", " + std::to_string(fWorldZPos) + "), material: " + worldMaterial + ", xSize: " + std::to_string(worldSzX) + ", ySize: " + std::to_string(worldSzY) + ", zSize: " + std::to_string(worldSzZ) + ", step limit: " + worldStepLimDesc);

    //Account for world-to-layers offset
    G4double layersOffset = 10.0 * mm;
    if (worldConf.layersOffset.has_value()) {
        layersOffset = worldConf.layersOffset.value() * mm;
    }
    Logger::debug("GEOMETRY", "Offset before layers: " + std::to_string(layersOffset));
    fCurrentZ = fWorldZPos - (worldSzZ / 2) + layersOffset;
}

void DetectorConstruction::AddLayer(const LayerConfig& layer) {
    // (Copy a single layer's placement logic from the loop body, using the `layer` parameter)
    // Update fCurrentZ and copy number as needed (keep state as class members)
    const auto& layerName = layer.name;
    if (std::find(fLayerNames.begin(), fLayerNames.end(), layerName) == fLayerNames.end()) {
        Logger::debug("GEOMETRY", "Adding layer name " + layerName + " to fLayerNames array(needed because most probably this method is being called by mac file, therefore the names array was not pre-populated)");
        fLayerNames.push_back(layerName);
    } else {
      if (! fGeomLoaded) {
            //raise exception as no name dups are allowed
            Logger::error("GEOMETRY", "Duplicate layer name detected: " + layerName + " (Each layer must have a unique name)");
            this->clear();
            //fLaXyers.cleXar();
            //fLayXerNames.clXear();
            throw std::runtime_error("Duplicate layer name detected: \"" + layerName + "\". Each layer must have a unique name.");
      }
    }
    G4Material* mat = fMaterialsManager->GetOrBuildMaterial(layer.materialName);

    G4double matDensity = mat->GetDensity();  // in kg/m3 (Geant4 base unit)
    matDensity = matDensity / (g/cm3);  // convert to g/cm3
    G4double weightPerSurfU = matDensity * layer.thickness; //Kg per m2
    //G4double g4LayerThickn = layer.thickness * mm;
    //Logger::info("GEOMETRY", "[DEBUG-PRESET] " + layerName + " " + std::to_string(g4LayerThickn));
    //fLayersEvalDataMap[layerName] = LayerEvalData{.density = matDensity, .weightPerSurfUnit = weightPerSurfU, .lThickness = g4LayerThickn};
    fLayersEvalDataMap[layerName] = LayerEvalData{.density = matDensity, .weightPerSurfUnit = weightPerSurfU};
    //Logger::info("GEOMETRY", "[DEBUG-SET] " + layerName + " fLayersEvalDataMap[layerName].lThickness): " + std::to_string(fLayersEvalDataMap[layerName].lThickness));
    
    //Logger::debug("GEOMETRY", "[WEIGHT][" + layerName + "] density: " + std::to_string(matDensity) + ", weightPerSurfUnit: " + std::to_string(weightPerSurfU));

    G4double halfThickness = layer.thickness / 2.0 * mm;
    //G4double dummyg4 = layer.thickness * mm;
    //Logger::info("GEOMETRY", "[DEBUG-SET] " + layerName + " fLayersEvalDataMap[layerName].lThickness): " + std::to_string(fLayersEvalDataMap[layerName].lThickness));
    //Logger::info("GEOMETRY", "[DEBUG2] " + std::to_string(dummyg4) + ", " + std::to_string(layer.thickness));
    auto solid = new G4Box(layerName, fCommonSzX / 2, fCommonSzY / 2, halfThickness);
    auto logic = new G4LogicalVolume(solid, mat, layerName);
    std::string stepLimitDesc;
    stepLimitDesc = "default";
    if (layer.maxStep.has_value()) {
        G4double stepLimit = layer.maxStep.value() * mm;
        G4UserLimits* userLimits = new G4UserLimits(stepLimit);
        logic->SetUserLimits(userLimits);
        stepLimitDesc = std::to_string(layer.maxStep.value()) + " mm";
    }
    const G4double layerXPos = fCommonXPos;
    const G4double layerYPos = fCommonYPos;
    G4double layerZPos = fCurrentZ + halfThickness;
    if (!Utils::isExcluded(layerName)) {
        fShieldThickness = fShieldThickness + layer.thickness;
        Logger::debug("GEOMETRY", "[" + layerName + "] fShieldThickness --> " + std::to_string(fShieldThickness));
        //Logger::info("GEOMETRY", "[" + layerName + "] fShieldThickness --> " + std::to_string(fShieldThickness));
    } else {
        Logger::debug("GEOMETRY", "[" + layerName + "] EXCLUDED from fShieldThickness increments");
        //Logger::info("GEOMETRY", "[" + layerName + "] EXCLUDED from fShieldThickness increments");
    }
    fKCurrCopyNumber++;
    fNumLayersAdded++;
    new G4PVPlacement(nullptr, {layerXPos, layerYPos, layerZPos}, logic, layerName, fLogicWorld, fKIsManyPlacement, fKCurrCopyNumber);
    Logger::debug("GEOMETRY", "Placing volume: " + layerName + " (layer n. " + std::to_string(fNumLayersAdded) + ", copy nbr: " + std::to_string(fKCurrCopyNumber) + ") at position (" + std::to_string(layerXPos) + ", " + std::to_string(layerYPos) + ", " + std::to_string(layerZPos) + "), material: " + layer.materialName + ", step limit: " + stepLimitDesc + ",  countAllEntr: " + std::to_string(layer.countAllEntrances) + ", trackPrimSurv: " + std::to_string(layer.trackPrimPartSurv) + ", reqResEnOnExit: " + std::to_string(layer.reqResidEnOnExit) + ", evalRadiations: " + std::to_string(layer.evalRad) + ", density: " + std::to_string(matDensity) + ", weightPerSurfUnit: " + std::to_string(weightPerSurfU));

   fCurrentZ += layer.thickness * mm;
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
    //G4NistManager* nist = G4NistManager::Instance();
    //const std::vector<G4String>& names = nist->GetNistMaterialNames();
    //for (const auto& name : names) {
    //    //G4cout << name << G4endl;
    //    Logger::info("GEOMETRY", "NISTMATERIAL: " + name);
    //}
    if (fGeomLoaded) {
        //this method would work also if called again, as the world, here, is re-created.
        AddWorld(fMaterialsManager->GetWorldConfig());

        // Build shield layers
        for (size_t i = 0; i < fLayers.size(); ++i) {
            const auto& layer = fLayers[i];
            AddLayer(layer);
        }

        // Add detector volume (air or vacuum) after last layer
        //AddDetector(fMaterialsManager->GetDetectorConfig());

        Logger::info("GEOMETRY", "Geometry loaded from configuration file. Origin: (" + std::to_string(fCommonXPos) + ", " + std::to_string(fCommonYPos) + ", " + std::to_string(fWorldZPos) + "), layers: " + std::to_string(fNumLayersAdded));
    } else {
        Logger::info("GEOMETRY", "Geometry possibly loaded from macro. Origin: (" + std::to_string(fCommonXPos) + ", " + std::to_string(fCommonYPos) + ", " + std::to_string(fWorldZPos) + "), layers: " + std::to_string(fNumLayersAdded));
    }
    fParticleDataManagerPtr->setGlobalGeomInfo(fLayersEvalDataMap, fShieldThickness);
    return fPhysWorld;
}

const std::vector<G4String>& DetectorConstruction::getLayerNames() const {
    return fLayerNames;
}

const std::vector<LayerConfig>& DetectorConstruction::GetLayerConfigs() const {
    return fLayers;
}

