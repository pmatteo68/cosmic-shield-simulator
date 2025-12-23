#include "MaterialsManager.hh"
#include "G4NistManager.hh"
#include <fstream>
#include <unordered_set>
#include <stdexcept>
#include "Logger.hh"

using json = nlohmann::json;

MaterialsManager::MaterialsManager(CustomMaterialLoader& custMatLoader): fCustMatLoader(custMatLoader) {}

void MaterialsManager::LoadFromJson(const std::string& filename) {
    Logger::debug("GEOMETRY", "Opening: " + filename);
    std::ifstream file(filename);
    if (!file.is_open()) {
        Logger::error("GEOMETRY", "Cannot open file: " + filename);
        //G4Exception("MaterialsManager::LoadFromJson", "FileError", FatalException, ("Cannot open file: " + filename).c_str());
        throw std::runtime_error("MaterialsManager::LoadFromJson - Cannot open file: " + filename);
    }

    json data;
    file >> data;

    Logger::debug("GEOMETRY", "Parsing world data.");
    const auto& worldData = data["world"];
    //worldMat = worldData["material"];
    worldConfig.materialName = worldData["material"];
    worldConfig.xSize =  worldData["xSize"];
    worldConfig.ySize =  worldData["ySize"];
    worldConfig.zSize =  worldData["zSize"];

    if (worldData.contains("position") && worldData["position"].is_array()) {
        auto& wpos = worldData["position"];
        worldConfig.position = G4ThreeVector(wpos[0].get<double>(), wpos[1].get<double>(), wpos[2].get<double>());
    }
    else {
        worldConfig.position = std::nullopt;
    }

    if (worldData.contains("layersOffset")) {
        worldConfig.layersOffset = worldData["layersOffset"];
    } else {
        worldConfig.layersOffset = std::nullopt;
    }
    if (worldData.contains("maxStep")) {
        worldConfig.maxStep = worldData["maxStep"];
    } else {
        worldConfig.maxStep = std::nullopt;
    }

    /*
    Logger::debug("GEOMETRY", "Parsing detector data.");
    const auto& detectorData = data["detector"];
    detectorConfig.materialName = detectorData["material"];
    //detectorConfig.thickness = detectorData["thickness"];
    if (detectorData.contains("thickness")) {
        detectorConfig.thickness = detectorData["thickness"];
    } else {
        detectorConfig.thickness = std::nullopt;
    }
    if (detectorData.contains("copyNbr")) {
        detectorConfig.copyNumber = detectorData["copyNbr"];
    } else {
        detectorConfig.copyNumber = std::nullopt;
    }*/

    Logger::debug("GEOMETRY", "Parsing layers data.");
    std::unordered_set<std::string> layerNamesSeen;
    for (const auto& item : data["layers"]) {
        LayerConfig config;

        const std::string layerName = item["name"];
        // Check for duplicate layer names
        if (!layerNamesSeen.insert(layerName).second) {
            Logger::error("GEOMETRY", "Duplicate layer name detected: " + layerName + " (Each layer must have a unique name)");
            throw std::runtime_error("Duplicate layer name detected: \"" + layerName + "\". Each layer must have a unique name.");
        }
        config.name = layerName; //item["name"];
        config.materialName = item["material"];
        config.thickness = item["thickness"];

        config.countAllEntrances = (item["countAllEntries"] == "true"); //true; //detector: false
        config.trackPrimPartSurv = (item["trackPrimSurvivors"] == "true"); //detector: true
        config.reqResidEnOnExit = (item["reqResidualEnrgOnExit"] == "true"); //detector: true

        if (item.contains("maxStep")) {
            //std::cout << "Raw JSON maxStep: " << item["maxStep"] << std::endl;
            config.maxStep = item["maxStep"];
        } else {
            config.maxStep = std::nullopt;
        }

        if (item.contains("evalRadiations")) {
            config.evalRad = (item["evalRadiations"] == "true"); //true; //detector: true
        } else {
            config.evalRad = false;
        }
        layers.push_back(config);
    }
}

WorldConfig MaterialsManager::GetWorldConfig() const {
    //WorldConfig config;
    //config.materialName = worldMat;  // Populate the WorldConfig object with data from worldMat
    return worldConfig;
}

//DetectorConfig MaterialsManager::GetDetectorConfig() const {
//    //DetectorConfig config;
//    //config.materialName = detectorMat;
//    return detectorConfig;
//}


//const std::string& MaterialsManager::getWorldMaterial() const {
//    return worldMat;
//}

const std::vector<LayerConfig>& MaterialsManager::GetLayerConfigs() const {
    return layers;
}

G4Material* MaterialsManager::GetOrBuildMaterial(const std::string& name) {
    Logger::debug("GEOMETRY", "Getting/Building material: " + name);
    if (materialCache.count(name)) return materialCache[name];

    G4NistManager* nist = G4NistManager::Instance();
    G4Material* mat = nist->FindOrBuildMaterial(name);
    if (!mat) {
        Logger::debug("GEOMETRY", "Material undefined in G4NIST database");
        mat = fCustMatLoader.getOrCreateMaterial(name);
        if (!mat) {
            Logger::error("GEOMETRY", "Unknown material: " + name);
            throw std::runtime_error("Unknown material: " + name);
        } else {
            Logger::info("GEOMETRY", "Material created (custom): " + name);
        }
    } else {
        Logger::info("GEOMETRY", "Material created (G4NIST): " + name);
    }
    materialCache[name] = mat;
    return mat;
}
