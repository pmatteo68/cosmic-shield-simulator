#ifndef SWMETA_HH
#define SWMETA_HH

#include <string>

class SWMeta {
private:
    static constexpr const char* TOOL_NAME     = "Cosmic Shield Simulator";
    static constexpr const char* TOOL_VERSION  = "5.1.5-250808";
    static constexpr const char* TOOL_AUTHOR   = "Matteo Picciau";
    //static constexpr const char* G4_VERSION    = G4Version; // defined in G4Version.hh

public:
    static std::string GetG4Version();
    static std::string GetToolName();
    static std::string GetVersion();
    static std::string GetAuthor();
    //static std::string GetInfo();  // desc
};

#endif

