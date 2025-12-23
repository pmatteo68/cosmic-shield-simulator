#include "Utils.hh"

#include <sstream>
#include <iomanip>
#include <string>
#include "G4Types.hh"
#include "G4Threading.hh"

std::string weightExcludLayers = std::getenv("CSS_WEIGHT_EXCLUSIONS") ? std::getenv("CSS_WEIGHT_EXCLUSIONS") : "Detector,detector";

std::string Utils::formatLayerLabel(int layerIdx) {
    return "L" + (std::ostringstream{} << std::setw(3) << std::setfill('0') << layerIdx).str();
}

//std::string Utils::formatDetectorLabel() {
//    return "LDET";
//}

std::string Utils::formatThreadLabel(G4int g4ThreadId) {
    //G4int g4ThreadId = G4Threading::G4GetThreadId();
    if (g4ThreadId >= 0) {
      return "G4WT" + (std::ostringstream{} << std::setw(3) << std::setfill('0') << g4ThreadId).str();
    } else {
      return "G4_MAIN";
    }
}

//bool Utils::isExcluded(const std::string& exclusionsCsv, const std::string& item) {
//    std::stringstream ss(exclusionsCsv);
//    std::string name;
//    while (std::getline(ss, name, ',')) {
//        if (name == item) return true;
//    }
//    return false;
//}

bool Utils::isExcluded(const std::string& item) {
    //std::stringstream ss(exclusionsCsv);
    std::stringstream ss(weightExcludLayers);
    std::string name;
    while (std::getline(ss, name, ',')) {
        if (name == item) return true;
    }
    return false;
}

