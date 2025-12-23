#include "ParticleDataManager.hh"
#include "Logger.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4Threading.hh"
#include "Utils.hh"
#include "SurvPrimCollector.hh"
#include "CustomCSVBuilder.hh"
//#include "CsvHelper.hh"

// Static members definition and initialization

std::string summGeomCsvSeparator = std::getenv("CSS_GLOB_GEOM_SUMM_CSV_SEP") ? std::getenv("CSS_GLOB_GEOM_SUMM_CSV_SEP") : ";";
std::string summGeomCsvFilename = std::getenv("CSS_GLOB_GEOM_SUMM_CSV_FNAME") ? std::getenv("CSS_GLOB_GEOM_SUMM_CSV_FNAME") : "summ_geom.csv";

std::string summPartCsvSeparator = std::getenv("CSS_GLOB_PART_SUMM_CSV_SEP") ? std::getenv("CSS_GLOB_PART_SUMM_CSV_SEP") : ";";
std::string summPartCsvFilename = std::getenv("CSS_GLOB_PART_SUMM_CSV_FNAME") ? std::getenv("CSS_GLOB_PART_SUMM_CSV_FNAME") : "summ_part.csv";

std::string globKpisCsvSeparator = std::getenv("CSS_GLOB_KPIS_CSV_SEP") ? std::getenv("CSS_GLOB_KPIS_CSV_SEP") : ";";
std::string globKpisCsvFilename = std::getenv("CSS_GLOB_KPIS_CSV_FNAME") ? std::getenv("CSS_GLOB_KPIS_CSV_FNAME") : "glob_kpis.csv";

//std::string weightExcludLayers = std::getenv("CSS_WEIGHT_EXCLUSIONS") ? std::getenv("CSS_WEIGHT_EXCLUSIONS") : "Detector,detector";

int kpi_layers_to_discard = std::getenv("CSS_KPI_DISCARDED_LAYERS") ? std::stoi(std::getenv("CSS_KPI_DISCARDED_LAYERS")) : 1;
std::string kpi_eval_direction = std::getenv("CSS_KPI_EVAL_DIRECTION") ? std::getenv("CSS_KPI_EVAL_DIRECTION") : "OUT";

//ParticleDataManager::ParticleDataManager(const std::vector<G4String>& layerNames): fLayerNames(layerNames) {
//ParticleDataManager::ParticleDataManager(DetectorConstruction& detConstruction, WRResolver& wrResol, SurvPrimCollector& survPrimColl, const G4bool radiationsON, const G4String& scopeDesc, const G4String& outputDataDir): fOutputDataDir(outputDataDir), fWrResolver(wrResol), fSurvPrimColl(survPrimColl), fWtNormSvON(radiationsON), fScopeDesc(scopeDesc) {
ParticleDataManager::ParticleDataManager(WRResolver& wrResol, SurvPrimCollector& survPrimColl, const G4bool radiationsON, const G4String& scopeDesc, const G4String& outputDataDir): fOutputDataDir(outputDataDir), fWrResolver(wrResol), fSurvPrimColl(survPrimColl), fWtNormSvON(radiationsON), fScopeDesc(scopeDesc) {
    Logger::debug("DATAMGR", "ParticleDataManager creation - Begin");
    //fLayersEvalDataMap = detConstruction.getLayersEvalDataMapCopy();
    //fGlobThickness = detConstruction.GetShieldThickness();
    //Logger::info("DATAMGR", "XXYY fLayersEvalDataMap size: " + std::to_string(fLayersEvalDataMap.size()));
    //Logger::info("DATAMGR", "XXYY fGlobThickness: " + std::to_string(fGlobThickness));
}

ParticleDataManager::~ParticleDataManager() {
    // Destructor body (currently empty)

    // testing for the track of the surviror primaries
    /*
    std::vector<std::string> particleNames = {"p1", "p2", "p3", "p4", "p5", "p6"};
    //// Loop N times
    for (int i = 0; i < 100000; ++i) {
        // Pick a random particle name
        int particleIndex = std::rand() % particleNames.size();
        std::string particleName = particleNames[particleIndex];

        // Pick an energy level based on the particle
        double energy = 0.0;
        //randMToN(double M, double N): return M + (rand() / ( RAND_MAX / (N-M) ) ) ;
        if (particleName == "p1" || particleName == "p2") {
            energy = (0.5 + (rand() / ( RAND_MAX / (1.0 - 0.5) ) ));
        } else if (particleName == "p3" || particleName == "p4") {
            energy = (5.0 + (rand() / ( RAND_MAX / (10.0 - 5.0) ) ));
        } else if (particleName == "p5" || particleName == "p6") {
            energy = (50.0 + (rand() / ( RAND_MAX / (100.0 - 50.0) ) )); 
        }
        //std::cout << "LIMBO Generated energy for " << particleName << ": " << energy << std::endl;

        // Add the particle to the collector
        fSurvPrimColl.AddSurvivor(1, particleName, energy);
    }
    fSurvPrimColl.Finalize();
    fSurvPrimColl.PrintSummary();*/
}

//void ParticleDataManager::setGlobalGeomInfo(DetectorConstruction& detConstruction) {
void ParticleDataManager::setGlobalGeomInfo(std::map<std::string, LayerEvalData> layersDataMap, double globalThickness) {
    fLayersEvalDataMap = layersDataMap; //detConstruction.getLayersEvalDataMapCopy();
    fGlobThickness = globalThickness; //detConstruction.GetShieldThickness();
    Logger::debug("DATAMGR", "fLayersEvalDataMap SET. size: " + std::to_string(fLayersEvalDataMap.size()));
    Logger::debug("DATAMGR", "fGlobThickness SET: " + std::to_string(fGlobThickness));
}

void ParticleDataManager::printExitingParticlesSummary() {
    std::lock_guard<std::mutex> lock(fMutex);
    if (!survPrimCollFinalized) {
        Logger::debug("DATAMGR", "Finalizing the survived particles collector");
        fSurvPrimColl.Finalize();
        survPrimCollFinalized = true;
    }
    fSurvPrimColl.PrintSummary();
}

//void ParticleDataManager::printGlobalKPIs(double beamInitEnrg, double beamInitRadPot, double shieldThickness) {
void ParticleDataManager::printGlobalKPIs(double beamInitEnrg, double beamInitRadPot) {
    std::vector<std::string> gkpiHeaders;
    std::stringstream ssgk;
    ssgk << "GlobThickness,GlobNormWeight,EnergyEfficiency";
    //bool includeRadiat = false;
    bool includeRadiat = fWtNormSvON;
    if (includeRadiat) {
        ssgk << ",ProtectionEfficiency";
    }

    std::string gkpiitem;
    while (std::getline(ssgk, gkpiitem, ',')) {
        gkpiHeaders.push_back(gkpiitem);
    }
    G4String fullGKPIsPathCsv = fOutputDataDir + "/" + globKpisCsvFilename;
    CustomCSVBuilder csvHelp = CustomCSVBuilder(globKpisCsvSeparator, gkpiHeaders, fullGKPIsPathCsv);
    bool withHead = true;
    csvHelp.open(withHead);

    //G4cout << "\n=== Global KPIs (" << ssgk.str() << ") ===" << G4endl;
    //G4cout << "\n=== Global KPIs (weight-excluded layers: " << weightExcludLayers << ", discarded layers: " << std::to_string(kpi_layers_to_discard) << ", eval. direction: " << kpi_eval_direction << ") ===" << G4endl;
    G4cout << "\n=== Global KPIs (discarded layers: " << std::to_string(kpi_layers_to_discard) << ", eval. direction: " << kpi_eval_direction << ") ===" << G4endl;
    KPIsConfig kpisCfg;
    kpisCfg.discarded_layers = kpi_layers_to_discard;
    kpisCfg.eval_direction = kpi_eval_direction;
    //double protEff = 0.0;

    DerivedGeomData geomData = evalGlobGeomData();
    double globalNrmWeight = geomData.globNormWeight;
    double globalThickn = geomData.globThickness;

    double enEff = evalEnergyEff(beamInitEnrg, kpisCfg);

    G4cout << "GLOBAL KPIs (GlobThickness) | " << globalThickn << G4endl;
    csvHelp.addDataField(globalThickn);
    G4cout << "GLOBAL KPIs (GlobNormWeight) | " << globalNrmWeight << G4endl;
    csvHelp.addDataField(globalNrmWeight);

    G4cout << "GLOBAL KPIs (EnergyEfficiency) | " << enEff << G4endl;
    csvHelp.addDataField(enEff);

    if (includeRadiat) {
        //beamInitRadPot
        double protEff = evalProtectionEff(beamInitRadPot, kpisCfg);
        G4cout << "GLOBAL KPIs (ProtectionEfficiency) | " << protEff << G4endl;
        csvHelp.addDataField(protEff);
    }
    G4cout << "======================================\n" << G4endl;
    csvHelp.close();
    Logger::info("DATAMGR", "Global KPIs have been saved to: " + fullGKPIsPathCsv);
}


void ParticleDataManager::initLayerDataMaps(const std::vector<G4String>& layerNames) {
    std::lock_guard<std::mutex> lock(fMutex);
    if (fMapsInitialized)
    {
        Logger::debug("DATAMGR", "ParticleDataManager::initLayerDataMaps - Maps initialized ALREADY - return");
        return;
    }

    fLayerNames = layerNames;
    for (const auto& layer : fLayerNames) {
        Logger::debug("DATAMGR", "ParticleDataManager::initLayerDataMaps - Initializing fDataPerVolumeAndType[" + layer + "] map for global summary");
        fDataPerVolumeAndType[layer];  // creates an empty inner map for this layer
    }
    fMapsInitialized = true;
}

void ParticleDataManager::trackSurvPrimary(const G4String& layer, G4int eventId, const G4String& particleName, G4double energy) {
    //std::lock_guard<std::mutex> lock(fMutex); no mutex as it is taken care
    //inside in the survivors collector
    fSurvPrimColl.AddSurvivor(layer, eventId, particleName, energy);
}

void ParticleDataManager::trackParticleIn(G4double kineticEnergy, const G4String& layer, const G4String& particleName) {
    std::lock_guard<std::mutex> lock(fMutex);
    auto& curLayer_Particle_Data = fDataPerVolumeAndType[layer][particleName];
    curLayer_Particle_Data.countIn += 1;
    curLayer_Particle_Data.energyIn += kineticEnergy;
    //fDataPerVolumeAndType[layer][particleName].countIn += 1;
    //fDataPerVolumeAndType[layer][particleName].energyIn += kineticEnergy;
}

void ParticleDataManager::trackParticleOut(G4double kineticEnergy, const G4String& layer, const G4String& particleName) {
    std::lock_guard<std::mutex> lock(fMutex);
    auto& curLayer_Particle_Data = fDataPerVolumeAndType[layer][particleName];
    curLayer_Particle_Data.countOut += 1;
    curLayer_Particle_Data.energyOut += kineticEnergy;
    //fDataPerVolumeAndType[layer][particleName].countOut += 1;
    //fDataPerVolumeAndType[layer][particleName].energyOut += kineticEnergy;
}

void ParticleDataManager::trackEnergyDeps(G4double depEnergy, G4double wtNrmEquivDose, const G4String& layer, const G4String& particleName) {
    std::lock_guard<std::mutex> lock(fMutex);
    auto& curLayer_Particle_Data = fDataPerVolumeAndType[layer][particleName];
    curLayer_Particle_Data.energyDep += depEnergy;
    if (fWtNormSvON) {
        curLayer_Particle_Data.wtNormEqDose += wtNrmEquivDose;
    }
}

const std::map<G4String, std::map<G4String, ParticleDataManager::ParticleDataRaw>>&
ParticleDataManager::GetData() const {
    return fDataPerVolumeAndType;
}

//double ParticleDataManager::evalGlobNormWeight() {
//    double glbNormWeight = 0.0;
//    for (const auto& volumeName : fLayerNames) {
//        double currNormWeight = 0.0;
//        if (! Utils::isExcluded(weightExcludLayers, volumeName)) {
//            LayerEvalData& layerDataMap = (fDetectorConstruction.getLayeXrsEvalDataMap())[volumeName];
//            double matDensity = layerDataMap.density;
//            currNormWeight = layerDataMap.weightPerSurfUnit;
//        }
//        glbNormWeight = glbNormWeight + currNormWeight;
//    }
//    Logger::info("DATAMGR", "[GLOBAL KPIs] Global normalized weight evaluated with exclusions: " + weightExcludLayers);
//    return (glbNormWeight);
//}

//double ParticleDataManager::evalGlobNormWeight() {
//    DerivedGeomData geomData = evXalGlobGeomData();
//    double glbNormWeight = geomData.globNormWeight;
//    return (glbNormWeight);
//}

//double ParticleDataManager::evalGlobThickness() {
//    DerivedGeomData geomData = evaXlGlobGeomData();
//    double glbThickness = geomData.globThickness;
//    return (glbThickness);
//}

DerivedGeomData ParticleDataManager::evalGlobGeomData() {
    double glbNormWeight = 0.0;
    //double glbThickness = fDetectorConstruction.GetShieldThickness();
    //auto& layersEvalDataMap = fDetectorConstruction.getLayersEvalDataMap();

    for (const auto& volumeName : fLayerNames) {
        double currNormWeight = 0.0;
        double currThickness = 0.0;
        //if (!Utils::isExcluded(weightExcludLayers, volumeName)) {
        if (!Utils::isExcluded(volumeName)) {
            //LayerEvalData& layerDataMap = (fDetectorConstruction.getLayerXsEvalDataMap())[volumeName];
            LayerEvalData& layerDataMap = fLayersEvalDataMap[volumeName]; //layersEvalDataMap[volumeName];
            currNormWeight = layerDataMap.weightPerSurfUnit;
            //currThickness = layerDataMap.lThickness;
            //Logger::info("DATAMGR", "[DEBUG:WGT] " + std::to_string(currNormWeight));
            //Logger::info("DATAMGR", "[DEBUG-POST] " + volumeName + " (fDetectorConstruction.getLayeXrsEvalDataMap())[volumeName].lThickness: " + std::to_string(currThickness));
        }
        glbNormWeight += currNormWeight;
        //glbThickness += currThickness;
        //Logger::info("DATAMGR", "[DEBUG:THICKNESS] [" + volumeName + "] TOTAL --> " + std::to_string(glbThickness));
    }
    //Logger::info("DATAMGR", "[GLOBAL KPIs] Global geometry data evaluated with exclusions: " + weightExcludLayers);
    Logger::info("DATAMGR", "[GLOBAL KPIs] Global geometry data evaluated");
    return {glbNormWeight, fGlobThickness};  //glbThickness};
}


void ParticleDataManager::printGeomEvalData() {
    std::vector<std::string> summGeomHeaders;
    //std::stringstream ssg("LayerIdx,Layer,Thickness,Density,NormWeight,CountIn,EnergyIn,CountOut,EnergyOut");
    std::stringstream ssg("LayerIdx,Layer,Density,NormWeight,CountIn,EnergyIn,CountOut,EnergyOut");
    std::string gitem;
    while (std::getline(ssg, gitem, ',')) {
        summGeomHeaders.push_back(gitem);
    }
    G4String fullGPathCsv = fOutputDataDir + "/" + summGeomCsvFilename;
    CustomCSVBuilder csvHelp = CustomCSVBuilder(summGeomCsvSeparator, summGeomHeaders, fullGPathCsv);
    bool withHead = true;
    csvHelp.open(withHead);

    //G4cout << "\n=== Geometry derived data (LayerLabel,LayerName,Thickness,Density,NormWeight,CountIn,EnergyIn,CountOut,EnergyOut) ===" << G4endl;
    G4cout << "\n=== Geometry derived data (LayerLabel,LayerName,Density,NormWeight,CountIn,EnergyIn,CountOut,EnergyOut) ===" << G4endl;
    int layerIdx = 0;
    //double gloXbNormWeight = 0.0;
    //auto& layersEvalDataMap = fDetectorConstruction.getLayersEvalDataMap();
    for (const auto& volumeName : fLayerNames) {
        layerIdx++;
        std::string layerLabel = Utils::formatLayerLabel(layerIdx);
        double matDensity = 0.0;
        double currNormWeight = 0.0;
        //double currThickness = 0.0;
        //if (! Utils::isExcluded(weightExcludLayers, volumeName)) {
        if (! Utils::isExcluded(volumeName)) {
            LayerEvalData& layerDataMap = fLayersEvalDataMap[volumeName]; //layersEvalDataMap[volumeName];
            matDensity = layerDataMap.density;
            currNormWeight = layerDataMap.weightPerSurfUnit;
            //currThickness = layerDataMap.lThickness;
        }
        //gloXbNormWeight = globXNormWeight + currNormWeight;
        //G4cout << layerLabel << " | " << volumeName << " | " << matDensity << " | " << currNormWeight << G4endl;
        csvHelp.addDataField(layerLabel);
        csvHelp.addDataField(volumeName);
        //csvHelp.addDataField(currThickness);
        csvHelp.addDataField(matDensity);
        csvHelp.addDataField(currNormWeight);

        double eIn = 0.0;
        double eOut = 0.0;
        int countIn = 0;
        int countOut = 0;
        auto it = fDataPerVolumeAndType.find(volumeName);
        if (! (it == fDataPerVolumeAndType.end())) {
            const auto& particleMap = it->second;
            for (const auto& particleEntry : particleMap) {
                const auto& particleName = particleEntry.first;
                const auto& data = particleEntry.second; // data.energyOut data.countOut data.energyIn data.countIn
                eIn = eIn + data.energyIn;
                eOut = eOut + data.energyOut;
                countIn = countIn + data.countIn;
                countOut = countOut + data.countOut;
            }
        }
        csvHelp.addDataField(countIn);
        csvHelp.addDataField(eIn);
        csvHelp.addDataField(countOut);
        csvHelp.addDataField(eOut);
        //G4cout << layerLabel << " | " << volumeName << " | " << currThickness << " | " << matDensity << " | " << currNormWeight << " | " << countIn << " | " << eIn << " | " << countOut << " | " << eOut << G4endl;
        G4cout << layerLabel << " | " << volumeName << " | " << matDensity << " | " << currNormWeight << " | " << countIn << " | " << eIn << " | " << countOut << " | " << eOut << G4endl;

    }
    //DerivedGeomData geomData = evalGXlobGeomData();
    ////double gloXbNormWeight = evalGlobNoXrmWeight();
    //double globalNormWeight = geomData.globNormWeight;
    //double globalThickness = geomData.globThickness;
    //G4cout << "ALL" << " | " << "ALL" << " | " << globalThickness << "|" << "n.a." << " | " << globalNormWeight << " | " << "n.a." << " | " << "n.a." << " | " << "n.a." << " | " << "n.a." << G4endl;
    G4cout << "======================================\n" << G4endl;
    //csvHelp.addDataField("ALL");
    //csvHelp.addDataField("ALL");
    //csvHelp.addDataField(globalThickness);
    //csvHelp.addDataField("n.a.");
    //csvHelp.addDataField(globalNormWeight);
    //csvHelp.addDataField("n.a.");
    //csvHelp.addDataField("n.a.");
    //csvHelp.addDataField("n.a.");
    //csvHelp.addDataField("n.a.");
    csvHelp.close();
    Logger::info("DATAMGR", "Geometry summary has been saved to: " + fullGPathCsv);
}

double ParticleDataManager::evalProtectionEff(const double beamRadiatPot, const KPIsConfig& config) {
    int discarded_layers = config.discarded_layers;
    std::string eval_direction = config.eval_direction;
    int num_layers = fLayerNames.size();
    double protEff = 0.0;
    double potentialRadiatPassed = 0.0;
    bool layerFound = false;
    size_t layerIdx = 0;

    for (const auto& volumeName : fLayerNames) {
        layerIdx++;
        std::string layerLabel = Utils::formatLayerLabel(layerIdx);
        Logger::debug("DATAMGR", "[GLOBAL KPIs] [Protection Eff.] Checking layer " + layerLabel + ": " + volumeName);
        if (layerIdx == (num_layers - discarded_layers)) {
            auto it = fDataPerVolumeAndType.find(volumeName);
            if (it == fDataPerVolumeAndType.end()) continue;  // Skip if not present
            const auto& particleMap = it->second;
            for (const auto& particleEntry : particleMap) {
                const auto& particleName = particleEntry.first;
                const auto& data = particleEntry.second;
                double currTypeTotEnergy = 0.0;
                int currTypeCount = 0;
                if (eval_direction == "OUT") {
                    currTypeTotEnergy = data.energyOut;
                    currTypeCount = data.countOut;
                } else {
                    currTypeTotEnergy = data.energyIn;
                    currTypeCount = data.countIn;
                }
                //full blown would be: potentialRadiatPassed = potentialRadiatPassed + currTypeCount * (currTypeTotEnergy / currTypeCount) * fWrResolver.ResolveWR(particleName, currTypeTotEnergy / currTypeCount);
                potentialRadiatPassed = potentialRadiatPassed + currTypeTotEnergy * fWrResolver.ResolveWR(particleName, (currTypeTotEnergy / currTypeCount));
            }
            Logger::info("DATAMGR", "[GLOBAL KPIs] Protection efficiency evaluated over reference layer #" + std::to_string(layerIdx) + ": " + volumeName);
            layerFound = true;
            break;
        }

    }
    if (layerFound && (beamRadiatPot > 0.0)) {
        protEff = 1.0 - (potentialRadiatPassed / beamRadiatPot);
    } else {
        Logger::error("DATAMGR", "[GLOBAL KPIs] Protection efficiency IMPOSSIBLE to calculate (either the reference layer was not found, or the beam's radiation potential was zero)");
    }

    return (protEff);
}

double ParticleDataManager::evalEnergyEff(const double beamEnergy, const KPIsConfig& config) {
    //const int discarded_layers, const std::string& eval_direction
    int discarded_layers = config.discarded_layers;
    std::string eval_direction = config.eval_direction;
    int num_layers = fLayerNames.size();
    double eEff = 0.0;
    double energyPassed = 0.0;
    bool layerFound = false;
    size_t layerIdx = 0;
    
    for (const auto& volumeName : fLayerNames) {
        layerIdx++;
        std::string layerLabel = Utils::formatLayerLabel(layerIdx);
        Logger::debug("DATAMGR", "[GLOBAL KPIs] [Energy Eff.] Checking layer " + layerLabel + ": " + volumeName);
        if (layerIdx == (num_layers - discarded_layers)) {
            auto it = fDataPerVolumeAndType.find(volumeName);
            if (it == fDataPerVolumeAndType.end()) continue;  // Skip if not present
            const auto& particleMap = it->second;
            //eXorSummary << "Layer (" << layerLabel << "): " << volumeName << "\n";
            //G4cout << "Layer (" << layXerLabel << "): " << volumXeName << G4endl;
            for (const auto& particleEntry : particleMap) {
                //const auto& particlXeName = particXleEntry.first;
                const auto& data = particleEntry.second;
                double currContrib = 0.0;
                if (eval_direction == "OUT") {
                    currContrib = data.energyOut;
                } else {
                    currContrib = data.energyIn;
                }
                energyPassed = energyPassed + currContrib;
            }
            Logger::info("DATAMGR", "[GLOBAL KPIs] Energy efficiency evaluated over reference layer #" + std::to_string(layerIdx) + ": " + volumeName);
            layerFound = true;
            break;
        }

    }
    if (layerFound && (beamEnergy > 0.0)) {
        eEff = 1.0 - (energyPassed / beamEnergy);
    } else {
        Logger::error("DATAMGR", "[GLOBAL KPIs] Energy efficiency IMPOSSIBLE to calculate (either the reference layer was not found, or the beam's energy was zero)");
    }
    return (eEff);
}

void ParticleDataManager::printPartSummary() {
    //std::vector<G4String> laxxyerNames = {"AlLayer", "PolyLayer", "WLayer"};
    std::string g4ThreadLabel = Utils::formatThreadLabel(G4Threading::G4GetThreadId());

    std::vector<std::string> summHeaders;
    std::stringstream ss("Thread,LayerIdx,Layer,Particle,In,Out,E_In,E_Out,E_Dep");
    std::string item;
    while (std::getline(ss, item, ',')) {
        summHeaders.push_back(item);
    }
    if (fWtNormSvON) {
        summHeaders.push_back("WTNEDose");
    }
    G4String fullPathCsv = fOutputDataDir + "/" + summPartCsvFilename;
    CustomCSVBuilder csvHelp = CustomCSVBuilder(summPartCsvSeparator, summHeaders, fullPathCsv);
    bool withHead = true;
    csvHelp.open(withHead);

    /*CsvHelper csvHelp;
    csvHelp.openFile(fOutputDataDir + "/summ.csv");
    std::vector<std::pair<G4String, char>> columns = {
        {"Thread", 'S'},
        {"Layer", 'S'},
        {"Particle", 'S'},
        {"In", 'D'},
        {"Out", 'D'},
        {"E_In", 'D'},
        {"E_Out", 'D'},
        {"E_Dep", 'D'}
    };
    if (fWtNormSvON) {
        columns.emplace_back("WTNEDose", 'D');
    }
    G4int tupleId = csvHelp.createTuple("summ", "Tuple for storing summary information", columns);*/

    //std::ostringstream eorSuXmmary;
    //G4cout << "====\n" << G4endl;
    //eorSuXmmary << "\n=== [G4Thread: " << g4ThreadLabel << "] Run ended. Summary by Layer and Particle (scope: global) ===\n";
    G4cout << "\n=== [G4Thread: " << g4ThreadLabel << "] Run ended. Summary by Layer and Particle (scope: " << fScopeDesc << ") ===" << G4endl;
    int layerIdx = 0;
    for (const auto& volumeName : fLayerNames) {
        layerIdx++;
        auto it = fDataPerVolumeAndType.find(volumeName);
        if (it == fDataPerVolumeAndType.end()) continue;  // Skip if not present

        const auto& particleMap = it->second;

        std::string layerLabel = Utils::formatLayerLabel(layerIdx);
        //eXorSummary << "Layer (" << layerLabel << "): " << volumeName << "\n";
        G4cout << "Layer (" << layerLabel << "): " << volumeName << G4endl;
        for (const auto& particleEntry : particleMap) {
            const auto& particleName = particleEntry.first;
            const auto& data = particleEntry.second;

            ////eorSuXmmary << "    Particle: " << particleName
            //eorSXummary << "    " << g4ThreadLabel << " | " << layerLabel << " | Particle: " << particleName
            G4cout << "    " << g4ThreadLabel << " | " << layerLabel << " | Particle: " << particleName
                    << " | In: " << data.countIn
                    << " | Out: " << data.countOut
                    << " | E_in: " << data.energyIn / MeV << " MeV"
                    << " | E_out: " << data.energyOut / MeV << " MeV"
                    << " | E_dep: " << data.energyDep / MeV << " MeV";
            csvHelp.addDataField(g4ThreadLabel);
            csvHelp.addDataField(layerLabel);
            csvHelp.addDataField(volumeName);
            csvHelp.addDataField(particleName);
            csvHelp.addDataField(data.countIn);
            csvHelp.addDataField(data.countOut);
            csvHelp.addDataField(data.energyIn);
            csvHelp.addDataField(data.energyOut);
            csvHelp.addDataField(data.energyDep);
            /*unsigned fIdx = 0;
            csvHelp.addDataField(fIdx++, g4ThreadLabel);
            csvHelp.addDataField(fIdx++, layerLabel);
            csvHelp.addDataField(fIdx++, particleName);
            csvHelp.addDataField(fIdx++, data.countIn);
            csvHelp.addDataField(fIdx++, data.countOut);
            csvHelp.addDataField(fIdx++, data.energyIn);
            csvHelp.addDataField(fIdx++, data.energyOut);
            csvHelp.addDataField(fIdx++, data.energyDep);*/

            if (fWtNormSvON) {
                G4cout << " | WTNEDose: " << data.wtNormEqDose << " Sv/WT";
                csvHelp.addDataField(data.wtNormEqDose);
                //csvHelp.addDataField(fIdx++, data.wtNormEqDose);
            }
            G4cout << G4endl;
        }
    }

    //eorSumXmary << "======================================\n";
    //G4cout << eorSXummary.str() << G4endl;
    G4cout << "======================================\n" << G4endl;

    csvHelp.close();
    Logger::info("DATAMGR", "Global summary has been saved to: " + fullPathCsv);
}

void ParticleDataManager::clear() {
    Logger::debug("DATAMGR", "Clearing global summary's data map");
    fDataPerVolumeAndType.clear();
    fSurvPrimColl.clear();
}

