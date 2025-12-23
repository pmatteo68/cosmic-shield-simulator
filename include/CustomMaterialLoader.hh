#pragma once

#include <string>
#include <map>
#include <vector>
#include "G4Material.hh"

struct ElementConfig {
    int Z;
    double A;
};

struct MaterialComponent {
    std::string elementName;
    double fraction;
};

struct MaterialConfig {
    double density;
    std::string unit;
    std::vector<MaterialComponent> components;
};

struct MaterialItem {
    G4Material* mat;
};

class CustomMaterialLoader {
public:
    CustomMaterialLoader();
    virtual ~CustomMaterialLoader();

    void clear();
    bool isAliasCached(const std::string& myName);
    bool definitionsLoaded();
    void init(const std::string& filePath);
    void addElement(const std::string& name, int Z, double A);
    void addMaterial(const std::string& name, double density, const std::string& unit,
                     const std::vector<MaterialComponent>& components);
    G4Material* getOrCreateMaterial(const std::string& name);

private:
    std::map<std::string, ElementConfig> elementMap_;
    std::map<std::string, MaterialConfig> materialMap_;

    int fNumAddedMaterialDefs = 0;
    int fNumAddedElemDefs = 0;
    bool fDefsLoaded = false;
    std::map<std::string, MaterialItem> fMaterialDefsCache;
};

