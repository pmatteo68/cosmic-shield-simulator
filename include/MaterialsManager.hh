#ifndef MATERIALS_MANAGER_HH
#define MATERIALS_MANAGER_HH

#include <vector>
#include <string>
#include <map>
#include "nlohmann/json.hpp"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include "CustomMaterialLoader.hh"

struct LayerConfig {
    std::string name;
    std::string materialName;
    double thickness; // in mm
    bool countAllEntrances;
    bool trackPrimPartSurv;
    bool reqResidEnOnExit;
    bool evalRad;
    std::optional<double> maxStep; // in mm
};

struct WorldConfig {
    std::optional<G4ThreeVector> position; //in m
    std::string materialName;
    double xSize; //in m
    double ySize; //in m
    double zSize; //in m
    std::optional<double> layersOffset; //in mm;
    std::optional<double> maxStep; // in mm
};

//struct DetectorConfig {
//    std::string materialName;
//    std::optional<double> thickness; // in mm
//    //double thickness; // in cm
//    std::optional<int> copyNumber;
//};

class MaterialsManager {
public:
    MaterialsManager(CustomMaterialLoader& custMatLoader);
    void LoadFromJson(const std::string& filename);

    const std::vector<LayerConfig>& GetLayerConfigs() const;
    G4Material* GetOrBuildMaterial(const std::string& name);

    const std::string& getWorldMaterial() const;  // Return a const reference to avoid unnecessary copies
    WorldConfig GetWorldConfig() const;
    //DetectorConfig GetDetectorConfig() const;

private:
    //std::string worldMat;
    WorldConfig worldConfig;
    //DetectorConfig detectorConfig;
    CustomMaterialLoader& fCustMatLoader;
    std::string detectorMat;
    std::vector<LayerConfig> layers;
    std::map<std::string, G4Material*> materialCache;
};

#endif
