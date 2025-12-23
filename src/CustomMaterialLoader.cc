#include "CustomMaterialLoader.hh"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include "Logger.hh"
#include "G4SystemOfUnits.hh"

using json = nlohmann::json;

CustomMaterialLoader::CustomMaterialLoader() {}

CustomMaterialLoader::~CustomMaterialLoader() {
}

void CustomMaterialLoader::clear()
{
    Logger::debug("CUSTMATLD", "Clearing the custom material loader");
    fDefsLoaded = false;
    fMaterialDefsCache.clear();
    fNumAddedMaterialDefs = 0;
    fNumAddedElemDefs = 0;
    materialMap_.clear();
    elementMap_.clear();
}

bool CustomMaterialLoader::isAliasCached(const std::string& myName) {
    //std::lock_guard<std::mutex> lock(fMutex);
    auto it = fMaterialDefsCache.find(myName);
    bool retVal = (it != fMaterialDefsCache.end());
    Logger::debug("CUSTMATLD", "Material alias '" + myName + "' isCached: " + std::to_string(retVal));
    return retVal;
}

bool CustomMaterialLoader::definitionsLoaded()
{
    //bool r = (fNumAddedMaterialDefs + fNumAddedElemDefs) > 0;
    Logger::debug("CUSTMATLD", "definitionsLoaded returns: " + std::to_string(fDefsLoaded));
    return fDefsLoaded;
}

void CustomMaterialLoader::init(const std::string& filePath) {
    Logger::debug("CUSTMATLD", "CustomMaterialLoader::init - trying to open file: " + filePath);
    std::ifstream file(filePath);
    if (!file.is_open()) { //throw std::runtime_error("Cannot open file: " + filePath);
        Logger::error("CUSTMATLD", "Failed to open file: " + filePath);
    }
    try {
        json j;
        file >> j;

        // Load elements
        for (const auto& e : j["elements"]) {
            std::string name = e.at("name");
            int Z = e.at("Z");
            double A = e.at("A");
            addElement(name, Z, A);
        }

        // Load materials
        for (const auto& m : j["materials"]) {
            std::string name = m.at("name");
            double density = m.at("density");
            std::string unit = m.at("unit");

            std::vector<MaterialComponent> components;
            for (const auto& c : m.at("components")) {
                components.push_back({c.at("element"), c.at("fraction")});
            }

            addMaterial(name, density, unit, components);
        }
        fDefsLoaded = (fNumAddedMaterialDefs + fNumAddedElemDefs) > 0;
    } catch (const std::exception& ex) {
        Logger::warn("CUSTMATLD", std::string("CustomMaterialLoader: definitions could not be loaded from configuration file: ") + ex.what());
        this->clear();
    }
}

void CustomMaterialLoader::addElement(const std::string& name, int Z, double A) {
    Logger::debug("CUSTMATLD", "Adding element definition: " + name + " (Z: " + std::to_string(Z) + ", A: " + std::to_string(A) + ")");
    if (elementMap_.count(name) > 0) {
        throw std::runtime_error("Duplicate element name: " + name);
    }
    elementMap_[name] = {Z, A};
    fNumAddedElemDefs++;
}

void CustomMaterialLoader::addMaterial(const std::string& name, double density,
                                        const std::string& unit,
                                        const std::vector<MaterialComponent>& components) {
    Logger::debug("CUSTMATLD", "Adding material definition: " + name + " (density: " + std::to_string(density) + ", unit: " + unit + ", components: " + std::to_string(components.size()));
    if (materialMap_.count(name) > 0) {
        throw std::runtime_error("Duplicate material name: " + name);
    }
    materialMap_[name] = {density, unit, components};
    fNumAddedMaterialDefs++;
}

G4Material* CustomMaterialLoader::getOrCreateMaterial(const std::string& name) {
    Logger::debug("CUSTMATLD", "CustomMaterialLoader::createMaterial - creating material: " + name);
    auto cachedit = fMaterialDefsCache.find(name);
    if (cachedit != fMaterialDefsCache.end()) {
        Logger::debug("CUSTMATLD", "G4Material returned from cache");
        return cachedit->second.mat;
    }

    auto it = materialMap_.find(name);
    if (it == materialMap_.end()) {
        throw std::runtime_error("Material not found: " + name);
    }

    const auto& mat = it->second;

    // Create elements (cache locally per call)
    std::map<std::string, G4Element*> localElements;
    int elcnt = 0;
    for (const auto& comp : mat.components) {
        elcnt++;
        const auto& elemName = comp.elementName;
        Logger::debug("CUSTMATLD", "Creating material's component[" + std::to_string(elcnt) + "]: " + elemName);
        auto eit = elementMap_.find(elemName);
        if (eit == elementMap_.end()) {
            throw std::runtime_error("Element not found: " + elemName);
        }
        const auto& ecfg = eit->second;
        localElements[elemName] = new G4Element(elemName, elemName, ecfg.Z, ecfg.A * g/mole);
        Logger::debug("CUSTMATLD", "Material's component[" + std::to_string(elcnt) + "] " + elemName + " G4Element created");
    }

    // Convert density
    double densityValue = mat.density;
    Logger::debug("CUSTMATLD", "Defining material's density (" + std::to_string(densityValue) + " " + mat.unit + ")");
    double density = 0.0;
    if (mat.unit == "g/cm3") {
        density = densityValue * g/cm3;
    } else if (mat.unit == "kg/m3") {
        density = densityValue * kg/m3;
    } else if (mat.unit == "mg/cm3") {
        density = densityValue * 0.001 * g/cm3;
    } else if (mat.unit == "g/mm3") {
        density = densityValue * 1000.0 * g/cm3;
    } else if (mat.unit == "g/m3") {
        density = densityValue * 1e-6 * g/cm3;
    } else {
        throw std::runtime_error("Unsupported density unit: " + mat.unit);
    }
    Logger::debug("CUSTMATLD", "Material's  density defined: " + std::to_string(density) + " " + mat.unit);

    Logger::debug("CUSTMATLD", "Creating G4Material");
    G4Material* g4mat = new G4Material(name, density, mat.components.size());
    int crcnt = 0;
    for (const auto& comp : mat.components) {
        crcnt++;
        Logger::debug("CUSTMATLD", "Adding component[" + std::to_string(crcnt) + "] G4Element to material");
        g4mat->AddElement(localElements[comp.elementName], comp.fraction);
    }
    //fMaterialDefsCache[name] = { mat };
    fMaterialDefsCache[name] = MaterialItem{ g4mat };
    Logger::debug("CUSTMATLD", "G4Material stored in cache");

    return g4mat;
}

