#include "BeamConfigManager.hh"
#include "Randomize.hh"
#include "Logger.hh"
#include <numeric>
#include <sstream>
#include <iomanip>
#include "G4SystemOfUnits.hh"
#include "CustomCSVBuilder.hh"

static const G4double minDirSz = 1e-6;

std::string beamBkdCsvSeparator = std::getenv("CSS_BEAMBKD_CSV_SEP") ? std::getenv("CSS_BEAMBKD_CSV_SEP") : ";";
std::string beamBkdCsvFilename = std::getenv("CSS_BEAMBKD_CSV_FNAME") ? std::getenv("CSS_BEAMBKD_CSV_FNAME") : "beam_bkd.csv";
int beamBkdCsvPctDigits = std::getenv("CSS_BEAMBKD_PCTDIGITS") ? std::stoi(std::getenv("CSS_BEAMBKD_PCTDIGITS")) : 2;

BeamConfigManager::BeamConfigManager(CustomIonLoader& custIonLoader, WRResolver& wrResol, const G4String& beamConfFile, const G4String& outputDataDir): fCustIonLoader(custIonLoader), fWrResolver(wrResol), fOutputDataDir(outputDataDir), fBeamConfFile(beamConfFile) {
    Logger::debug("BEAMCFG", "BeamConfigManager::BeamConfigManager - Begin");
    fMessenger = std::make_unique<BeamConfigMessenger>(this);
}

BeamConfigManager::~BeamConfigManager() {
}

G4ParticleDefinition* BeamConfigManager::createCustomParticleDef(const std::string& myParticleName)
{
    return (fCustIonLoader.createParticleDef(myParticleName));
}

bool BeamConfigManager::custPartDefsLoaded()
{
    return (fCustIonLoader.definitionsLoaded());
}

void BeamConfigManager::AddCustomParticleDef(const std::string& myName, int Z, double A, double excitationEnergy) {
    fCustIonLoader.addParticleDefinition(myName, Z, A, excitationEnergy);
}

void BeamConfigManager::AddParticle(const G4String& name,
                             G4double energy,
                             G4double flux,
                             const G4ThreeVector& position,
                             const G4ThreeVector& momDir) {
                             //const G4ThreeVector& defxMomdir) {
    std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("BEAMCFG","BeamConfigManager::AddParticle - BEGIN");
    G4int i_cur_count = fBeamComposition.size();
    G4ThreeVector dir = momDir;
    if (dir.mag() < minDirSz) {
        std::ostringstream errMsg;
        dir = fDefMomentumDir; //defMXomdir;
        errMsg << "Momentum direction vector is too small (<" << minDirSz << "). Defaulting to ("
            << dir.x() << ", "
            << dir.y() << ", "
            << dir.z() << ").";
        Logger::error("BEAMCFG", errMsg.str().c_str());
        G4Exception("AddParticle", "BadMomentum", JustWarning,
                   errMsg.str().c_str());
    }
    dir = dir.unit();
    G4ThreeVector positAdj = position * m;
    BeamParticle bParticle{name, energy, flux, positAdj, dir};
    std::ostringstream debugMsg;
    debugMsg << "[adding beam particle definition][" << i_cur_count << "] " << name << " - Energy: " << energy << ", Flux: " << flux
             << ", Position: ("
             << positAdj.x() << ", "
             << positAdj.y() << ", "
             << positAdj.z() << "), Normalized Mom. Dir.: ("
             << dir.x() << ", "
             << dir.y() << ", "
             << dir.z() << ")";
    Logger::debug("BEAMCFG", debugMsg.str().c_str());
    fBeamComposition.push_back(bParticle);
    fParticleUseCounts.push_back(0);
    fTotalFlux += flux;
    Logger::debug("BEAMCFG", "BeamConfigManager::AddParticle - BeamParticle ADDED to array, totalFlux: " + std::to_string(fTotalFlux));
}

G4bool BeamConfigManager::isConfigPreloaded() const
{
    Logger::debug("BEAMCFG", "BeamConfigManager::isConfigPreloaded - returning " + std::to_string(fIsConfigLoadedFromFile));
    return (fIsConfigLoadedFromFile);
}

BeamParticle BeamConfigManager::SelectParticle() {
    std::lock_guard<std::mutex> lock(fMutex);
    //Logger::debug("BEAMCFG", "BeamConfigManager::SelectParticle - Begin");
    if (fBeamComposition.empty()) {
        G4Exception("BeamConfig::SelectParticle", "EmptyBeam", JustWarning,
                    "No particles in beam composition");
        Logger::warn("BEAMCFG", "BeamConfigManager::SelectParticle - No particles in beam composition, returning default-initialized BeamParticle");
        return {}; // returns default-initialized BeamParticle
    }

    //// SiXmple uniXform selecXtion
    //G4int rndIndex = G4RaXndFlat::shoXotInt(fBeaXmComposition.size());
    // Flux-based selection
    G4double r = G4UniformRand() * fTotalFlux;
    G4double accum = 0.0;
    size_t rndIndex = 0;
    for (size_t i = 0; i < fBeamComposition.size(); ++i) {
        accum += fBeamComposition[i].flux;
        if (r <= accum) {
            rndIndex = i;
            break;
        }
    }
    // Fallback on numeric error
    G4bool fallbackHappened = false;
    if (r > accum) {
        fallbackHappened = true;
        fFallbackSelectionsCount++;
        rndIndex = fBeamComposition.size() - 1;
    }

    fParticleUseCounts[rndIndex]++;
    fSelectionsCount++;
    //Logger::debug("BEAMCFG", "BeamConfigManager::SelectParticle - picking particle[" + std::to_string(rndIndex) + "] by G4RandFlat::shootInt (count->" + std::to_string(fParticleUseCounts[rndIndex]) + ")");
    Logger::debug("BEAMCFG", "BeamConfigManager::SelectParticle(" + std::to_string(fSelectionsCount) + ") - picking particle[" + std::to_string(rndIndex) + "] by G4UniformRand + flux-based randomic selection (count->" + std::to_string(fParticleUseCounts[rndIndex]) + ", fallback: " + std::to_string(fallbackHappened) + ")");
    return fBeamComposition[rndIndex];
}

double BeamConfigManager::GetBeamInitEnergy() const {
    double enIn = 0.0;
    int numBeamComponents = fBeamComposition.size();
    for (size_t i = 0; i < numBeamComponents; ++i) {
        const auto& p = fBeamComposition[i];
        G4int curUsed = fParticleUseCounts[i];
        enIn = enIn + (curUsed * p.energy);
    }
    return (enIn);
}

double BeamConfigManager::GetBeamInitRadiationPotential() const {
    double radPotIn = 0.0;
    int numBeamComponents = fBeamComposition.size();
    for (size_t i = 0; i < numBeamComponents; ++i) {
        const auto& p = fBeamComposition[i];
        G4int curUsed = fParticleUseCounts[i];
        radPotIn = radPotIn + (curUsed * p.energy * fWrResolver.ResolveWR(fCustIonLoader.resolveAlias(p.name), p.energy));
    }
    return (radPotIn);
}

void BeamConfigManager::ReportParticleUsage() const {

    G4int totUsed = std::accumulate(fParticleUseCounts.begin(), fParticleUseCounts.end(), 0);

    std::vector<std::string> beamBkdHeaders;
    std::stringstream ss("Particle,Alias,isCustom,Energy,Flux,Count,Pct");
    std::string item;
    while (std::getline(ss, item, ',')) {
        beamBkdHeaders.push_back(item);
    }
    G4String csvFullPath = fOutputDataDir + "/" + beamBkdCsvFilename;
    CustomCSVBuilder csvHelp = CustomCSVBuilder(beamBkdCsvSeparator, beamBkdHeaders, csvFullPath);
    bool withHead = true;
    csvHelp.open(withHead);

    G4cout << "[BEAMCOMP] === Beam Composition Breakdown (total: " << totUsed << ", fallbacks: " << fFallbackSelectionsCount << ") ===" << G4endl;
    for (size_t i = 0; i < fBeamComposition.size(); ++i) {
        const auto& p = fBeamComposition[i];
        G4int curUsed = fParticleUseCounts[i];
        G4double curPct = 100 * curUsed / totUsed;
        std::string curPctAsStr = (std::ostringstream{} << std::fixed << std::setprecision(beamBkdCsvPctDigits) << curPct).str();
        G4cout << "[BEAMCOMP]  [" << curUsed << ", " << curPctAsStr << "%] " << p.name
               << ", " << p.energy << " MeV"
               << ", flux: " << p.flux << G4endl;
        csvHelp.addDataField(fCustIonLoader.resolveAlias(p.name));
        csvHelp.addDataField(p.name);
        csvHelp.addDataField(fCustIonLoader.isAliasCached(p.name) ? "Y" : "N"); //A ? "Y" : "N"
        csvHelp.addDataField(p.energy);
        csvHelp.addDataField(p.flux);
        csvHelp.addDataField(curUsed);
        csvHelp.addDataField(curPctAsStr);
    }
    csvHelp.close();
    Logger::info("BEAMCFG", "Beam particles breakdown has been saved to: " + csvFullPath);
}

// beamBkdCsvSeparator = std::getenv("CSS_BEAMBKD_CSV_SEP") ? std::getenv("CSS_BEAMBKD_CSV_SEP") : ";";
//std::string beamBkdCsvFilename = std::getenv("CSS_BEAMBKD_CSV_FNAME") ? std::getenv("CSS_BEAMBKD_CSV_FNAME") : "beam_bkd.csv";



void BeamConfigManager::LoadConfigFromFile() {
    try {
        fBeamConfig = GetJsonConfigHandle();
    }
    catch (const std::exception& ex) {
        Logger::warn("BEAMCFG", std::string("BeamConfigManager: beam composition could not be loaded from file: ") + ex.what());
        return;
    }

    //defining the default particle initial position
    if (fBeamConfig.contains("def_position")) {
        auto def_pos = fBeamConfig["def_position"];
        fDefPosition = G4ThreeVector(def_pos[0], def_pos[1], def_pos[2]);
    }
    Logger::debug("BEAMCFG", "fDefPosition: SET");

    // defining the default momentum direction
    if (fBeamConfig.contains("def_momentum_direction")) {
        auto def_mom = fBeamConfig["def_momentum_direction"];
        fDefMomentumDir = G4ThreeVector(def_mom[0], def_mom[1], def_mom[2]);
    }
    Logger::debug("BEAMCFG", "fDefMomentumDir: SET");

    //const auto& dXata = fBeamXConfig;
    //int prtidx = 0;
    for (const auto& item : fBeamConfig["beam"]) {
        //prtidx++;

        //defining position
        G4ThreeVector partPos;
        if (item.contains("position")) {
            auto pos = item["position"];
            //partPos = G4ThreeVector(pos[0], pos[1], pos[2]) * m;
            partPos = G4ThreeVector(pos[0], pos[1], pos[2]);
        } else {
            //partPos = fDefPosition * m;
            partPos = fDefPosition;
        }

        //defining momentum direction
        G4ThreeVector dir = fDefMomentumDir;
        if (item.contains("momentum_direction")) {
            auto mom = item["momentum_direction"];
            dir = G4ThreeVector(mom[0], mom[1], mom[2]);
        }

        //name, energy, flux, position, momentumDir
        this->AddParticle(item["particle"], item["energy"], item["flux"], partPos, dir);
    }
    G4int entriesAdded = fBeamComposition.size();
    if (entriesAdded > 0) {
        fIsConfigLoadedFromFile = true;
    }
    Logger::debug("BEAMCFG", "fIsConfigLoadedFromFile set to " + std::to_string(fIsConfigLoadedFromFile) + " after loading from json (added particle entries: " + std::to_string(entriesAdded) + ")");
}

json BeamConfigManager::GetJsonConfigHandle() const {
    std::string beamConfigPath = fBeamConfFile;
    Logger::debug("BEAMCFG", "BeamConfigManager::LoadBeamConfig - Opening beam file " + beamConfigPath + " ...");
    std::ifstream file(beamConfigPath);
    if (!file.is_open()) {
        Logger::error("BEAMCFG", "BeamConfigManager::LoadBeamConfig - FileError - Cannot open beam config file " + beamConfigPath);
        //G4Exception("BeamConfigManager::LoadBeamConfig", "FileError", FatalException, ("Cannot open beam file: " + beamConfigPath).c_str());
        throw std::runtime_error("BeamConfigManager::GetJsonConfigHandle - Cannot open file: " + beamConfigPath);
    }

    json data;
    file >> data;
    Logger::debug("BEAMCFG", "Beam configuration LOADED (from: " + beamConfigPath + ")");
    //G4cxxout << "[DEBUG][BEAMCFG] Beam configuration LOADED (from: " << bexamConfigPath << ")" << G4endl;
    return data;  // Return the parsed JSON
}

