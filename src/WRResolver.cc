#include "WRResolver.hh"
#include <fstream>
#include <nlohmann/json.hpp>
#include <G4String.hh>
#include "Logger.hh"

using json = nlohmann::json;

// Static member definition and initialization
static const bool debugWrRes = []() {
    const char* env = std::getenv("CSS_WRRES_DEBUG");
    return (env && std::strcmp(env, "ON") == 0);
}();

WRResolver::WRResolver(const G4String& wmfilepath) {
    LoadFromFile(wmfilepath);
}

void WRResolver::LoadFromFile(const G4String& wmfilepath) {
    std::ifstream file(wmfilepath);
    if (!file) {
        Logger::error("WRRESLV", "The WR Resolver configuration file (" + wmfilepath + ") could not be opened, defaults will apply");
        return;
    }

    json j;
    file >> j;

    if (j.contains("defaultWr")) {
        fDefaultWr = j["defaultWr"];
        Logger::debug("WRRESLV", "[setup] Default WR: " + std::to_string(fDefaultWr));
    } else {
        Logger::debug("WRRESLV", "[setup] Default WR (embedded): " + std::to_string(fDefaultWr));
    }
    const auto& array = j["wrRules"];
    int idx=0;
    for (const auto& entry : array) {
        WRRule curRule;
        curRule.ruleId = idx;
        Logger::debug("WRRESLV", "[setup]    Loading rule #" + std::to_string(curRule.ruleId));
        curRule.particName = entry["partName"].get<std::string>();
        Logger::debug("WRRESLV", "[setup]        partName: " + curRule.particName);        

        if (entry.contains("enLow")) {
            curRule.enLow = entry["enLow"].get<G4double>();
            Logger::debug("WRRESLV", "[setup]        enLow: " + std::to_string(entry["enLow"].get<G4double>()));
        }
        if (entry.contains("enHigh")) {
            curRule.enHigh = entry["enHigh"].get<G4double>();
            Logger::debug("WRRESLV", "[setup]        enHigh: " + std::to_string(entry["enHigh"].get<G4double>()));
        }
        curRule.wr = entry["wr"].get<G4double>();
        fWRRules.push_back(curRule);
        idx++;
    }
    Logger::info("WRRESLV", "[setup] Nbr of WR rules loaded (from " + wmfilepath + "): " + std::to_string(idx));
}

G4double WRResolver::ResolveWR(const G4String& particleName, G4double energy) const {
    for (const auto& curRule : fWRRules) {
        if (curRule.particName != particleName)
            continue;
        if (curRule.enLow.has_value() && energy < curRule.enLow.value())
            continue;
        if (curRule.enHigh.has_value() && energy >= curRule.enHigh.value())
            continue;

        if (debugWrRes) {
            Logger::debug("WRRESLV", "(" + particleName + ", " + std::to_string(energy) + ") -> WRRULE " + std::to_string(curRule.ruleId) + " -> " + std::to_string(curRule.wr));
        }
        return curRule.wr;
    }

    // Default WR if no match
    if (debugWrRes) {
        Logger::debug("WRRESLV", "(" + particleName + ", " + std::to_string(energy) + ") -> DEFAULT WR");
    }
    return (fDefaultWr);
}

