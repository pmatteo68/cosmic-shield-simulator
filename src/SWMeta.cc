#include "SWMeta.hh"
#include "G4Version.hh"
#include <string>
//#include <iostream>

std::string SWMeta::GetG4Version() {
    std::string fullVersion = G4Version; //"$Name: geant4-11-03-patch-01 [MT]$";
    
    // Find the position of the version string (after the "Name: " part)
    size_t startPos = fullVersion.find("Name: ") + 6;  // 6 is the length of "Name: "
    
    // Find the position of the space before [MT] (end of the version string)
    size_t endPos = fullVersion.find(" [MT]");

    // Extract the version substring
    return fullVersion.substr(startPos, endPos - startPos);
    //return G4Version;  // G4Version is a G4String compatibile with std::string
}

std::string SWMeta::GetToolName() {
    return std::string(TOOL_NAME);
}

std::string SWMeta::GetVersion() {
    return std::string(TOOL_VERSION);
}

std::string SWMeta::GetAuthor() {
    return std::string(TOOL_AUTHOR);
}

//std::string SWMeta::GetInfo() {
//    return std::string("[INFO] ") + TOOL_NAME + " " + TOOL_VERSION + "\n"
//         + "[INFO] Author: " + TOOL_AUTHOR + "\n"
//         + "[INFO] Using Geant4: " + GetGeant4Version() + "\n";
//}

