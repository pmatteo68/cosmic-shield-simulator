// PhysicsManager.cc
#include "PhysicsManager.hh"
#include "G4PhysListFactory.hh"
#include "Logger.hh" // Assuming you have a Logger class
#include <typeinfo>

#include "QGSP_BIC_AllHP.hh"
#include "QGSP_BERT_HP.hh"
#include "FTFP_BERT_HP.hh"
#include "Shielding.hh"
#include "QGSP_INCLXX_HP.hh"
#include "QGSP_BIC_HP.hh"

PhysicsManager::PhysicsManager(int verbosity, const std::string& physListName)
  : fVerbosity(verbosity), fPhysListName(physListName) {}

G4VModularPhysicsList* PhysicsManager::createPhysList() {
    Logger::info("DRIVER", "Loading physics list: " + fPhysListName);

    G4VModularPhysicsList* physicsList = nullptr;
    const std::string staticMsg = "Physics list is being loaded STATICALLY (initialization verbosity: " + std::to_string(fVerbosity) + ")";

    if (fPhysListName == "QGSP_BIC_AllHP") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new QGSP_BIC_AllHP(fVerbosity);
    } else if (fPhysListName == "QGSP_BERT_HP") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new QGSP_BERT_HP(fVerbosity);
    } else if (fPhysListName == "FTFP_BERT_HP") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new FTFP_BERT_HP(fVerbosity);
    } else if (fPhysListName == "Shielding") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new Shielding(fVerbosity);
    } else if (fPhysListName == "QGSP_INCLXX_HP") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new QGSP_INCLXX_HP(fVerbosity);
    } else if (fPhysListName == "QGSP_BIC_HP") {
        Logger::info("PHYSMGR", staticMsg);
        physicsList = new QGSP_BIC_HP(fVerbosity);
    } else {
        Logger::info("PHYSMGR", "Physics list is being loaded dynamically (initialization verbosity: unsettable)");
        G4PhysListFactory factory;

        if (!factory.IsReferencePhysList(fPhysListName)) {
            Logger::error("PHYSMGR", "Invalid physics list: " + fPhysListName);
            return nullptr;
        }

        physicsList = factory.GetReferencePhysList(fPhysListName);
    }

    if (physicsList)
        Logger::info("PHYSMGR", std::string("Physics list LOADED: ") + typeid(*physicsList).name());

    return physicsList;
}

