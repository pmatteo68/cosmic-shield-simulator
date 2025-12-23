#ifndef PHYSICSMANAGER_HH
#define PHYSICSMANAGER_HH

#include "G4VModularPhysicsList.hh"
#include <string>

class PhysicsManager {
public:
    PhysicsManager(int verbosity, const std::string& physListName);
    G4VModularPhysicsList* createPhysList();

private:
    int fVerbosity;
    std::string fPhysListName;
};

#endif // PHYSICSMANAGER_HH

