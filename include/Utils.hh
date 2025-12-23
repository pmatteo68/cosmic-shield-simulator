#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include "G4Threading.hh"

class Utils {
public:
    static std::string formatLayerLabel(int layerIdx);
    static std::string formatThreadLabel(G4int g4ThreadId);
    //static bool isExcluded(const std::string& exclusionsCsv, const std::string& item);
    static bool isExcluded(const std::string& item);
    //static std::string formatDetectorLabel();
};

#endif
