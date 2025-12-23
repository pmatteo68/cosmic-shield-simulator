#ifndef G4META_HH
#define G4META_HH

#include <string>

class G4MetaDataUtils {
private:
    //static constexpr const char* G4_VERSION = G4Version; // defined in G4Version.hh

public:
    static void DumpMaterialsDb(const std::string* matDbBasePath);
};

#endif

