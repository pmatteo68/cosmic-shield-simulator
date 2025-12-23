#ifndef CUSTOM_ION_LOADER_HH
#define CUSTOM_ION_LOADER_HH

#include "G4ParticleDefinition.hh"
#include <map>
#include <string>
//#include <mutex>

class CustomIonLoader {
public:
    CustomIonLoader();
    virtual ~CustomIonLoader();

    void addParticleDefinition(const std::string& myName, int Z, double A, double excitationEnergy);
    void initFromFile(const std::string& filePath);
    G4ParticleDefinition* createParticleDef(const std::string& myName);
    void clear();
    bool definitionsLoaded();
    std::string resolveAlias(const std::string& myName);
    bool isAliasCached(const std::string& myName);

private:
    struct IonParams {
        int Z;
        double A;
        double excitationEnergy;
    };
    struct ParticleItem {
        std::string g4Name;
        G4ParticleDefinition* particleDef;
    };

    std::map<std::string, IonParams> fDefinitions;
    int fNumAddedDefinitions = 0;
    bool fDefsLoaded = false;
    //mutable std::mutex fMutex;

    std::map<std::string, ParticleItem> fParticleDefsCache;
};

#endif // CUSTOM_ION_LOADER_HH

