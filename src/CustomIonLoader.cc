#include "CustomIonLoader.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"

#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "Logger.hh"

using json = nlohmann::json;

CustomIonLoader::CustomIonLoader() {}

CustomIonLoader::~CustomIonLoader() {
}

void CustomIonLoader::addParticleDefinition(const std::string& myName, int Z, double A, double excitationEnergy) {
    //std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("CUSTOMIONLD", "Trying to create ion definition parameters set (" + myName + ") with Z=" + std::to_string(Z) + ", A=" + std::to_string(A) + ", exc. energy=" + std::to_string(excitationEnergy));
    if (fDefinitions.find(myName) != fDefinitions.end()) {
        throw std::runtime_error("Duplicate ion name in map: " + myName);
    }
    Logger::debug("CUSTOMIONLD", "Passed uniqueness test");
    IonParams params;
    params.Z = Z;
    params.A = A;
    params.excitationEnergy = excitationEnergy;
    
    fDefinitions[myName] = params;
    fNumAddedDefinitions++;
    Logger::debug("CUSTOMIONLD", "Added particle definition parameters set: " + myName);

}

void CustomIonLoader::clear() {
    //std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("CUSTOMIONLD", "Clearing custom particle definitions");
    fDefinitions.clear();
    fDefsLoaded = false;
    fNumAddedDefinitions = 0;
}

bool CustomIonLoader::definitionsLoaded()
{
    //std::lock_guard<std::mutex> lock(fMutex);
    //bool defsLoaded = (fNumAddedDefinitions > 0);
    Logger::debug("CUSTOMIONLD", "areDefinitionsLoaded - returning " + std::to_string(fDefsLoaded));
    return (fDefsLoaded);
}

bool CustomIonLoader::isAliasCached(const std::string& myName) {
    //std::lock_guard<std::mutex> lock(fMutex);
    auto it = fParticleDefsCache.find(myName);
    bool retVal = (it != fParticleDefsCache.end());
    Logger::debug("CUSTOMIONLD", "Particle alias '" + myName + "' isCached: " + std::to_string(retVal));
    return retVal;
}

std::string CustomIonLoader::resolveAlias(const std::string& myName) {
    //std::lock_guard<std::mutex> lock(fMutex);
    auto it = fParticleDefsCache.find(myName);
    if (it != fParticleDefsCache.end()) {
        Logger::debug("CUSTOMIONLD", "Particle alias resolved: " + myName  + " -> " + it->second.g4Name);
        return it->second.g4Name;
    }
    Logger::debug("CUSTOMIONLD", "Particle alias resolved: " + myName  + " -> " + myName);
    return myName;
}


void CustomIonLoader::initFromFile(const std::string& filePath) {
    //std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("CUSTOMIONLD", "Loading custom particle definitions from: " + filePath);
    std::ifstream file(filePath);
    if (!file.is_open()) {
        //throw std::runtime_error("Failed to open file: " + filePath);
        Logger::error("CUSTOMIONLD", "Failed to open file: " + filePath);
    }
    else {
        //Logger::debug("CUSTOMIONLD", "Passed file opening");
        try {
            json j;
            file >> j;

            const auto& defs = j.at("particleDefs");
            for (const auto& entry : defs) {
                std::string name = entry.at("name");
                int Z = entry.at("Z");
                double A = entry.at("A");
                double excit = entry.at("excitationEnergy");
                //Logger::debug("CUSTOMIONLD", "Before addParticleDefinition");
                addParticleDefinition(name, Z, A, excit);
            }
            fDefsLoaded = (fNumAddedDefinitions > 0);
        } catch (const std::exception& ex) {
            Logger::warn("CUSTOMIONLD", std::string("CustomIonLoader: definitions could not be loaded from configuration file: ") + ex.what());
            this->clear();
        }
    }
}

G4ParticleDefinition* CustomIonLoader::createParticleDef(const std::string& myName) {
    Logger::debug("CUSTOMIONLD", "Creating custom particle definition for: " + myName);
    auto it = fDefinitions.find(myName);
    if (it != fDefinitions.end()) {
        Logger::debug("CUSTOMIONLD", "Custom particle '" + myName + "' parameters set FOUND in memory");
        IonParams params = it->second;
        int Z = params.Z;
        double A = params.A;
        double excitEn = params.excitationEnergy;

        auto cacheItem = fParticleDefsCache.find(myName);
        ParticleItem* itemPtr = (cacheItem != fParticleDefsCache.end()) ? &cacheItem->second : nullptr;
        if (itemPtr) {
             Logger::debug("CUSTOMIONLD", "Custom particle '" + myName + "' definition FOUND in cache");
             return itemPtr->particleDef;
        } else {
            Logger::debug("CUSTOMIONLD", "Custom particle '" + myName + "' definition NOT FOUND in cache");
            ParticleItem newCacheItem;
            Logger::debug("CUSTOMIONLD", "Custom particle '" + myName + "' creation ongoing ...");
            G4ParticleDefinition* p = G4IonTable::GetIonTable()->GetIon(Z, A, excitEn);
            if (!p) {
                //Logger::error("CUSTOMIONLD", "Failed to create particle definition (" + myName + ") with Z=" + std::to_string(Z) + ", A=" + std::to_string(A), ", exc. en.: " + std::to_string(excitEn));
                Logger::error("CUSTOMIONLD", "Failed to create particle definition (" + myName + ")");
            } else {
                Logger::debug("CUSTOMIONLD", "Custom particle '" + myName + "' definition CREATED");
                newCacheItem.particleDef = p;
                newCacheItem.g4Name = std::string(p->GetParticleName().c_str());
                fParticleDefsCache[myName] = newCacheItem;
                Logger::debug("CUSTOMIONLD", "Particle definition cached (" + myName + ") with Z=" + std::to_string(Z) + ", A=" + std::to_string(A) + ", exc. en.: " + std::to_string(excitEn) + ", G4 name: " + p->GetParticleName());
            }
            return p;
        }
    } else {
        Logger::warn("CUSTOMIONLD", "Custom particle parameters set not found for: " + myName);
    }
    return nullptr;
}

