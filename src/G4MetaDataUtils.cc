#include "G4MetaDataUtils.hh"

#include "G4Material.hh"
#include "G4SystemOfUnits.hh"
#include <nlohmann/json.hpp>
#include "G4NistManager.hh"
#include <fstream>
#include <iomanip>
#include <string>

#include "Logger.hh"


//This function has NOT to be called in the ordinary flow of simulation.
//It serves the purpose of dumping the Geant4 materials database, but, once done, the memory is so inflated that the program needs to be closed.
//Refer README, also main cc file, and also dump*.sh script for additional details.
void G4MetaDataUtils::DumpMaterialsDb(const std::string* matDbBasePath) {
    if (!matDbBasePath) {
        Logger::debug("G4MDATA", "G4 Materials database dumping NOT REQUIRED - skip");
        return;
    }

    G4NistManager* nist = G4NistManager::Instance();
    const std::vector<G4String>& names = nist->GetNistMaterialNames();

    Logger::info("G4MDATA", std::string("Dumping Geant4 materials database (base path: ") + (matDbBasePath ? *matDbBasePath : "<none>") + ")");

    const std::string txtFilePath = *matDbBasePath + "_list.txt";
    //std::ofstream outtxt(*matDbBasePath + "_list.txt");
    std::ofstream outtxt(txtFilePath); // this truncates/clears the file on open
    if (outtxt.is_open()) {
        for (const auto& name : names) {
            outtxt << name << '\n';
        }
        outtxt.close();
    }
    Logger::info("G4MDATA", "Geant4 materials database DUMPED (" + txtFilePath + ", format: TXT)");

    nlohmann::json j;
    j["materials"] = nlohmann::json::array();

    for (const auto& name : names) {
        G4Material* mat = nist->FindOrBuildMaterial(name, false);
        if (!mat) continue;

        double density_g_per_cm3 = mat->GetDensity() / (g/cm3);

        nlohmann::json mat_entry;
        mat_entry["matname"] = std::string(name);
        mat_entry["matdensity"] = density_g_per_cm3;
        j["materials"].push_back(mat_entry);
    }

    const std::string jsonFilePath = *matDbBasePath + "_db.json";
    //std::ofstream outjson(*matDbBasePath + "_db.json");
    std::ofstream outjson(jsonFilePath);
    outjson << std::setw(2) << j << std::endl;
    outjson.close();
    Logger::info("G4MDATA", "Geant4 materials database DUMPED (" + jsonFilePath + ", format: JSON)");
}


