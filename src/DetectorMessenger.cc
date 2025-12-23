#include "DetectorMessenger.hh"
#include "DetectorConstruction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "CustomMaterialLoader.hh"
#include "Logger.hh"

DetectorMessenger::DetectorMessenger(DetectorConstruction* detector)
: fDetector(detector)
{
    std::string baseCmd = "/css/geom";

    fDetDir = new G4UIdirectory((baseCmd + "/").c_str());
    fDetDir->SetGuidance("Commands for custom geometry construction");

    fAddWorldCmd = new G4UIcmdWithAString((baseCmd + "/addWorld").c_str(), this);
    fAddWorldCmd->SetGuidance("Add world from string");

    fAddLayerCmd = new G4UIcmdWithAString((baseCmd + "/addLayer").c_str(), this);
    fAddLayerCmd->SetGuidance("Add layer from string");

    std::string materialsBaseCmd = baseCmd + "/materials";
    fAddElementDefCmd = new G4UIcmdWithAString((materialsBaseCmd + "/elements/addDef").c_str(), this);
    fAddElementDefCmd->SetGuidance("Add element definition from string");

    fAddMaterialDefCmd = new G4UIcmdWithAString((materialsBaseCmd + "/addDef").c_str(), this);
    fAddMaterialDefCmd->SetGuidance("Add material definition from string");
}

DetectorMessenger::~DetectorMessenger() {
    delete fAddMaterialDefCmd;
    delete fAddElementDefCmd;

    delete fAddWorldCmd;
    delete fAddLayerCmd;
    delete fDetDir;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
    //G4bool isGeomPreloaded = fDetector.isGeomPreloaded();
    if (fDetector->isGeomPreloaded()) {
        Logger::warn("MACRO", "This command cannot be executed in this context (geometry has been already defined)");
        return;
    }

    CustomMaterialLoader& custMatLoader = fDetector->getCustMatLoader();
    bool areCustMatsPreloaded = custMatLoader.definitionsLoaded();

    if (command == fAddWorldCmd) {
        std::istringstream iss(newValue);
        WorldConfig wConfig;
        G4double x = 0.0, y = 0.0, z = 0.0;
        std::string material = "G4_Galactic";
        G4double xSize = 1.0, ySize = 1.0, zSize = 2.0;
        G4double offset = 10.0;
        G4double maxStep = -1.0;
        iss >> x >> y >> z;
        wConfig.position = G4ThreeVector(x, y, z);
        if (!(iss >> material)) material = "G4_Galactic";
        wConfig.materialName = material;
        if (iss >> xSize) wConfig.xSize = xSize;
        if (iss >> ySize) wConfig.ySize = ySize;
        if (iss >> zSize) wConfig.zSize = zSize;
        if (iss >> offset) wConfig.layersOffset = offset;
        if (iss >> maxStep) wConfig.maxStep = maxStep;
        // Only set optional maxStep if the user provided it
        if (maxStep > 0) {
            wConfig.maxStep = maxStep;
        } else {
            wConfig.maxStep.reset();
        }
        fDetector->AddWorld(wConfig);
    } else if (command == fAddLayerCmd) {
        std::istringstream iss(newValue);
        LayerConfig lConfig;
        std::string aStr, bStr, cStr, dStr;
        iss >> lConfig.name >> lConfig.materialName >> lConfig.thickness >> aStr >> bStr >> cStr >> dStr;
        lConfig.countAllEntrances = (aStr == "true");
        lConfig.trackPrimPartSurv = (bStr == "true");
        lConfig.reqResidEnOnExit = (cStr == "true");
        lConfig.evalRad = (dStr == "true");
        G4double maxStep = -1.0;
        if (iss >> maxStep && maxStep > 0.0) {
            lConfig.maxStep = maxStep;
        } else {
            lConfig.maxStep.reset();
        }
        fDetector->AddLayer(lConfig);
    } else if (command == fAddElementDefCmd) {
        if (areCustMatsPreloaded) {
            Logger::warn("MACRO", "This command cannot be executed in this context (custom materials have been already defined)");
            return;
        }
        int Z;
        double A;
        G4String custElemame;
        std::istringstream iss(newValue);
        iss >> std::quoted(custElemame) >> Z >> A;
        custMatLoader.addElement(custElemame, Z, A);
       
    } else if (command == fAddMaterialDefCmd) {
        if (areCustMatsPreloaded) {
            Logger::warn("MACRO", "This command cannot be executed in this context (custom materials have been already defined)");
            return;
        }
        std::istringstream iss(newValue);
        std::string token;
        iss >> token; // /css/geom/materials/addDef - skip command prefix
        std::string name, unit;
        double density;
        iss >> name >> density >> unit;
        std::vector<MaterialComponent> components;
        Logger::debug("MACRO", "Parsed material's name, density, unit: " + name + ", " + std::to_string(density) + ", " + unit);
        int iComp = 0;
        while (iss >> token) {
            iComp++;
            size_t sep = token.find(':');
            if (sep == std::string::npos) {
                throw std::runtime_error("Invalid material component format: " + token);
            }
            std::string elemName = token.substr(0, sep);
            double fraction = std::stod(token.substr(sep + 1));
            Logger::debug("MACRO", "Parsed material's component[" + std::to_string(iComp) + "], element: " + elemName + ", fraction: " + std::to_string(fraction));
            components.push_back({ elemName, fraction });
        }

        custMatLoader.addMaterial(name, density, unit, components);
    }

}

