// ------------------------------
// FILE: CosmicShieldSimulation.cc
// ------------------------------

//#include "G4Version.hh"
#include "CmdLineParser.hh"
#include "SWMeta.hh"
#include "AppConfig.hh"
//#include "G4PhysListFactory.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
//#include "G4SteppingVerbose.hh"

#include "DetectorConstruction.hh"
#include "BeamConfigManager.hh"
//#include "PrimaryGeneratorAction.hh"
//#include "RunAction.hh"
//#include "SteppingAction.hh"
//the following complies to the multi-threaded model
#include "ActionInitialization.hh"
#include "ParticleDataManager.hh"
//#include "CsvMerger.hh"
#include "Logger.hh"
#include "PhysicsManager.hh"
#include "WRResolver.hh"
#include "SurvPrimCollector.hh"
#include "CustomIonLoader.hh"
#include "CustomMaterialLoader.hh"
#include "StepDataWriter.hh"
#include "G4MetaDataUtils.hh"

namespace fs = std::filesystem;

//#include "QGSP_BIC_AllHP.hh"

// Static members definition and initialization

int plInitVerbosity = (std::getenv("CSS_PHYSLIST_VERBOSITY") && std::stringstream(std::getenv("CSS_PHYSLIST_VERBOSITY")) >> plInitVerbosity) ? plInitVerbosity : 0;

//bool csvMeXrgingEnabled = []() {
//    const char* env = std::getenv("CSS_CSV_MExRGING");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool radiationsON = []() {
//    const char* env = std::getenv("CSS_EVAL_RADIATIONS");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool saveStepData = []() {
//    const char* env = std::getenv("CSS_SAVE_STEPDATA");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool survPartRepON = []() {
//    const char* env = std::getenv("CSS_SURVIVORPART_SUMMARY");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//applies to step data csv
G4double zChartOffset = (std::getenv("CSS_CHART_Z_OFFSET") && std::stringstream(std::getenv("CSS_CHART_Z_OFFSET")) >> zChartOffset) ? zChartOffset : 0;

G4int survPartclsBinSz = (std::getenv("CSS_SURV_PTC_BINSIZE") && std::stringstream(std::getenv("CSS_SURV_PTC_BINSIZE")) >> survPartclsBinSz) ? survPartclsBinSz : 10;

//if true, the step data csv has ALL fields
bool isEnhancedStepdataCsv = []() {
    const char* env = std::getenv("CSS_STEPDATA_CSV_ENHANCED");
    return (env && std::strcmp(env, "ON") == 0);
}();

//if specified, the Geant4 NIST materials database is dumped, both in TXT and json format, for utilization with the OPTIMIZER
const std::string* g4MaterialsDbBasePath = []() -> const std::string* {
    const char* env = std::getenv("CSS_G4_MATERIALS_BPATH");
    if (env && *env != '\0') {
        static const std::string value(env);
        return &value;
    }
    return nullptr;
}();

//const char* g4MatXerialsDbBasePath = []() -> const char* {
//    const char* env = std::getenv("CSS_G4_MATERIALS_BPATH");
//    return (env && *env != '\0') ? env : nullptr;
//}();

//bool mergXerInfersHeader = []() {
//    const char* env = std::getenv("CSS_CSV_INxFER_HEADER");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool globPartSummON = []() {
//    const char* env = std::getenv("CSS_GLOBAL_PART_SUMMARY");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool repPartUsageON = []() {
//    const char* env = std::getenv("CSS_BEAM_BREAKDOWN");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

//bool threadPartSummON = []() {
//    const char* env = std::getenv("CSS_THREAD_PART_SUMMARY");
//    return (env && std::strcmp(env, "ON") == 0);
//}();

int main(int argc, char** argv) {
    CmdLineParser cmdLinParser(argc, argv, "css-");
    int logLevel = std::stoi(cmdLinParser.getParamValue("css-log-level", "6"));
    Logger::setLevel(logLevel);

    std::string appConfFile = cmdLinParser.getParamValue("css-conf-app", "./config/appconfig.json");
    std::string geomConfFile = cmdLinParser.getParamValue("css-conf-geom", "./config/geometry.json");
    std::string materialsConfFile = cmdLinParser.getParamValue("css-conf-materials", "./config/custom-materials.json");
    std::string particlesConfFile = cmdLinParser.getParamValue("css-conf-particles", "./config/custom-particles.json");
    std::string beamConfFile = cmdLinParser.getParamValue("css-conf-beam", "./config/beam.json");
    std::string wrResolConfFile = cmdLinParser.getParamValue("css-conf-wrres", "./config/wrresolver.json");

    //// Verbose stepping (optional)
    // G4SteppingVerbose::UseBestUnit verbose;
    Logger::info("DRIVER", SWMeta::GetToolName() + " v. " + SWMeta::GetVersion() + " (by: " + SWMeta::GetAuthor() + ")");
    Logger::info("DRIVER", "Using Geant4 v. " + SWMeta::GetG4Version());

    Logger::debug("DRIVER", "Creating RunManager (default)");
    G4RunManager* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    //const char* appCXonfigDirEnv = std::getenv("CSS_CONFIG_DIR");
    //std::string appCXonfigDir = appCoXnfigDirEnv ? appCXonfigDirEnv : "./config";
    //Logger::info("DRIVER", "Using application configuration directory: " + appXConfigDir);
    Logger::info("DRIVER", "Using application configuration: " + appConfFile);
    Logger::info("DRIVER", "Using geometry configuration: " + geomConfFile);
    Logger::info("DRIVER", "Using materials configuration: " + materialsConfFile);
    Logger::info("DRIVER", "Using particles configuration: " + particlesConfFile);
    Logger::info("DRIVER", "Using beam configuration: " + beamConfFile);
    Logger::info("DRIVER", "Using WR resolution configuration: " + wrResolConfFile);

    //std::string appConXfigPath = apXpConfigDir + "/appconfig.json";
    //AppConfig appxConfig(appCxonfigPath);
    AppConfig appConfig(appConfFile);

    //Define output dir
    //std::string reporXtsDir = std::getenv("CSxS_OUT_DIR") ? std::getenv("CSxS_OUT_DIR") : ".";
    std::string reportsDir = cmdLinParser.getParamValue("css-output-dir", "./out");

    //Output generation
    bool saveStepData = (cmdLinParser.getParamValue("css-save-stepdata", "OFF") == "ON");
    bool threadPartSummON = (cmdLinParser.getParamValue("css-thread-part-report", "OFF") == "ON");
    bool globGeomSummON = (cmdLinParser.getParamValue("css-global-geom-report", "ON") == "ON");
    bool globPartSummON = (cmdLinParser.getParamValue("css-global-part-report", "ON") == "ON");
    bool repPartUsageON = (cmdLinParser.getParamValue("css-beam-breakdown", "ON") == "ON");
    bool survPartRepON = (cmdLinParser.getParamValue("css-survivors-summary", "ON") == "ON");
    bool radiationsON = (cmdLinParser.getParamValue("css-eval-radiations", "ON") == "ON");
    bool isStepDataCsvG4Impl = (cmdLinParser.getParamValue("css-stepdata-csv-impl", "G4") == "G4");
    bool saveGlobalKPIs = (cmdLinParser.getParamValue("css-global-kpis", "ON") == "ON");

    // Set mandatory initialization classes

    //Custom materials loader
    Logger::debug("DRIVER", "Creating CustomMaterialLoader");
    CustomMaterialLoader* custMatLoader = new CustomMaterialLoader();
    custMatLoader->init(materialsConfFile);

    //DetectorConstruction
    Logger::debug("DRIVER", "Creating DetectorConstruction");
    DetectorConstruction* detConstr = new DetectorConstruction(geomConfFile, *custMatLoader);

    Logger::debug("DRIVER", "Creating CustomIonLoader");
    CustomIonLoader* custIonLoader = new CustomIonLoader();
    //custIonLoader->initFromFile(appXConfigDir + "/custom-particles.json");
    custIonLoader->initFromFile(particlesConfFile);

    Logger::debug("DRIVER", "Creating WRResolver, conf: " + wrResolConfFile);
    WRResolver wrRes = WRResolver(wrResolConfFile);

    Logger::debug("DRIVER", "Creating BeamConfigManager");
    //BeamConfigManager* beamConfManager = new BeamConfigManager(*custIonLoader, appXConfigDir, reportsDir);
    BeamConfigManager* beamConfManager = new BeamConfigManager(*custIonLoader, wrRes, beamConfFile, reportsDir);
    beamConfManager->LoadConfigFromFile();

    Logger::debug("DRIVER", "Setting user initialization (Detector Construction)");
    runManager->SetUserInitialization(detConstr);

    auto physListName = appConfig.GetPhysicsListName();
    PhysicsManager physManager(plInitVerbosity, physListName);
    G4VModularPhysicsList* physicsList = physManager.createPhysList();

    //Logger::debug("DRIVER", "Setting user initialization (Physics list)");
    runManager->SetUserInitialization(physicsList);
    Logger::info("DRIVER", "RunManager initialized with physics list.");

    //create datadir if needed
    bool outDirNeeded = (saveStepData || globGeomSummON || globPartSummON || repPartUsageON || saveGlobalKPIs);
    if (outDirNeeded) {
        Logger::info("DRIVER", "Creating datafiles directory: " + reportsDir);
        //G4cout << "[INFO] Creating datafiles directory: " << reportsDir << G4endl;
        fs::create_directories(reportsDir);
        Logger::info("DRIVER", "Chart Z-offset: " + std::to_string(zChartOffset));
    } else {
        Logger::info("DRIVER", "No output data will be saved (by configuration)");
    }

    Logger::info("DRIVER", "Global KPIs: " + std::string(saveGlobalKPIs ? "enabled" : "disabled"));
    Logger::info("DRIVER", "Global geometry summary: " + std::string(globGeomSummON ? "enabled" : "disabled"));
    Logger::info("DRIVER", "Global particles summary: " + std::string(globPartSummON ? "enabled" : "disabled"));
    Logger::info("DRIVER", "Thread particles summary: " + std::string(threadPartSummON ? "enabled" : "disabled"));

    //These constants and the content of script bin/printRawStepDataFPatt.sh
    //are interrelated. Modify only if fully aware of the impacts.
    //const std::string& stepDatafExt = "csv";
    //const std::string& baseStepDataFName = "data";
    //const std::string& stepDataTupleName = "step";
    std::string stepDatafExt = std::getenv("ST_DATA_FILE_EXT") ? std::getenv("ST_DATA_FILE_EXT") : "csv";
    std::string baseStepDataFName = std::getenv("ST_DATA_FILEBASENAME") ? std::getenv("ST_DATA_FILEBASENAME") : "data";
    std::string stepDataTupleName = std::getenv("ST_DATA_TUPLE_NAME") ? std::getenv("ST_DATA_TUPLE_NAME") : "step";


    const std::string& repBaseFName = baseStepDataFName + "." + stepDatafExt;
    //const std::string& custStepDataFileName = "daXta_nt_stXep_t0.cXsv";
    const std::string custStepDataFileName = baseStepDataFName + "_" + stepDataTupleName + "." + stepDatafExt;
    Logger::debug("DRIVER", "Creating SurvPrimCollector, binSz: " + std::to_string(survPartclsBinSz));
    SurvPrimCollector survPColl = SurvPrimCollector(survPartclsBinSz, reportsDir);
    Logger::debug("DRIVER", "Creating ParticleDataManager");
    ParticleDataManager partDMGR = ParticleDataManager(wrRes, survPColl, radiationsON, "global", reportsDir);
    detConstr->setParticleDataManagerRef(partDMGR);
    //ParticleDataManager partDMGR = ParticleDataManager(*detConstr, wrRes, survPColl, radiationsON, "global", reportsDir);

    //StepDataWriter* stepDataWr;
    //if (saveStepData) {
    StepDataWriter stepDataWr = StepDataWriter(reportsDir, custStepDataFileName, saveStepData, zChartOffset, isEnhancedStepdataCsv, radiationsON);
    if (saveStepData && (!isStepDataCsvG4Impl)) {
        stepDataWr.open(true);
        Logger::info("DRIVER", "Step data csv generation: CUSTOM (single file, concurrent writes)");
    }
    //}
    Logger::debug("DRIVER", "Setting user initialization (ActionInitialization)");
    runManager->SetUserInitialization(new ActionInitialization(stepDataWr, partDMGR, wrRes, *detConstr, *beamConfManager, reportsDir, repBaseFName, saveStepData, zChartOffset, isEnhancedStepdataCsv, globPartSummON, threadPartSummON, radiationsON, isStepDataCsvG4Impl, stepDataTupleName));
    Logger::debug("DRIVER", "SetUserInitialization: done");

    // Visualization manager
    Logger::debug("DRIVER", "Creating visualization manager");
    G4VisManager* visManager = new G4VisExecutive();
    Logger::debug("DRIVER", "Initializing visualization manager");
    visManager->Initialize();
    Logger::debug("DRIVER", "visManager->Initialize(): done");

    // Get the UI manager
    Logger::debug("DRIVER", "Getting UI pointer");
    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    G4String runMacOption = "css-mac";
    G4String defRunMac = "./run.mac";
    bool uiMode = cmdLinParser.optionIsSpecified("css-ui");
    G4String exeCommand = "/control/execute ";

    if (g4MaterialsDbBasePath) {
        Logger::info("DRIVER", "*** This run will NOT launch any simulation as a request to dump the G4 materials database has been made by configuration, and in this case the program will exit RIGHT AFTER this has been done. This is expected behavior, as the G4 materials database dumping functionality inflates memory in a non-recoverable way. ***");
        G4MetaDataUtils::DumpMaterialsDb(g4MaterialsDbBasePath);
        Logger::info("DRIVER", "EXIT (this is expected behavior, as the G4 materials database dumping functionality inflates memory in a non-recoverable way)");
        return 0;
    }

    //partDMGR.setGlobalGeomInfo(*detConstr);
    //if (argc == 1) {
    if (uiMode) {
        // Interactive mode
        Logger::info("DRIVER", "Starting in INTERACTIVE mode");
        Logger::debug("DRIVER", "Creating UI Executive");
        //I remove, from the arguments passed to G4UIExecutive,
        // ALL the arguments with 'css-'. As the Executive object
        // would fail with non-G4 arguments.
        // So this is my trick to introduce some custom command line parameters, like --css:ui for instance
        int g4argc = cmdLinParser.getG4Argc();
        char** g4argv = cmdLinParser.getG4Argv();
        G4UIExecutive* ui = new G4UIExecutive(g4argc, g4argv);
        G4String uiInitUiMacro = cmdLinParser.getParamValue("css-vismac", "./init_vis.mac");
        G4String uicommand = exeCommand + uiInitUiMacro;
        Logger::info("DRIVER", "Applying UI Manager Command: " + uicommand);
        UImanager->ApplyCommand(uicommand);
        G4String uiRunMacro  = cmdLinParser.getParamValue(runMacOption, defRunMac);
        uicommand = exeCommand + uiRunMacro;
        Logger::info("DRIVER", "Applying UI Manager Command: " + uicommand);
        UImanager->ApplyCommand(uicommand);
        Logger::debug("DRIVER", "Starting UI session");
        ui->SessionStart();
        delete ui;
    } else {
        // Batch mode
        Logger::info("DRIVER", "Starting in BATCH mode");
        G4String fileName = cmdLinParser.getParamValue(runMacOption, defRunMac);
        Logger::info("DRIVER", "Applying batch command: " + exeCommand + fileName);
        UImanager->ApplyCommand(exeCommand + fileName);
    }

    Logger::debug("DRIVER", "Deleting visualization manager");
    delete visManager;
    Logger::debug("DRIVER", "Deleting run manager");
    delete runManager;

    //wrap up and summaries
    stepDataWr.close();
    if (repPartUsageON) {
        beamConfManager->ReportParticleUsage(); 
    }
    if (saveGlobalKPIs) {
        double beamInEnrg = beamConfManager->GetBeamInitEnergy();
        double beamInRadPot = 0.0;
        if (radiationsON) {
            beamInRadPot = beamConfManager->GetBeamInitRadiationPotential();
        }
        //double shieldThickn = detConstr->GetShieldThickness();
        //partDMGR.printGlobalKPIs(beamInEnrg, beamInRadPot, shieldThickn);
        partDMGR.printGlobalKPIs(beamInEnrg, beamInRadPot);
    }
    if (globGeomSummON) {
        partDMGR.printGeomEvalData();
    }
    if (globPartSummON) {
        partDMGR.printPartSummary();
        if (survPartRepON) {
            partDMGR.printExitingParticlesSummary();
        }
    }
    if (globGeomSummON || globPartSummON || saveGlobalKPIs) {
        partDMGR.clear();
    }

    Logger::info("DRIVER", "Simulator processing COMPLETE.");

    //the CsvMerger class works, but it's way too heavy as it relates to memory, in presence of high volumes, so I deactivate this call for safety, as of 3.8.2
    /*
    if (saveStepData && csvMergingEnabled) {
      std::vector<int> keyFields = CsvMerger::envVarToIntVector("CSS_CSV_KEYFIELDS", {0, 1, 2});
      std::vector<int> sortFields = CsvMerger::envVarToIntVector("CSS_CSV_SORTBY", keyFields);
      int csvMergeThreads = CsvMerger::envVarAsInt("CSV_MERGER_THREADS", 4);

      //if repBaseFName is 'data.csv' this pattern is 'data*.csv'
      std::string repFileNamePattern = repBaseFName.find('.') != std::string::npos
          ? repBaseFName.substr(0, repBaseFName.find_last_of('.')) + "*" + repBaseFName.substr(repBaseFName.find_last_of('.'))
          : repBaseFName + "*";
      //const char inColDelimit = ',';
      //const char outColDelimit = ';';
      const char inColDelimit = (std::getenv("CSS_CSVMERGER_IN_COLSEP") && *std::getenv("CSS_CSVMERGER_IN_COLSEP")) ? std::getenv("CSS_CSVMERGER_IN_COLSEP")[0] : ',';
      const char outColDelimit = (std::getenv("CSS_CSVMERGER_OUT_COLSEP") && *std::getenv("CSS_CSVMERGER_OUT_COLSEP")) ? std::getenv("CSS_CSVMERGER_OUT_COLSEP")[0] : ';';

      CsvMerger::merge(
        reportsDir,     // Input folder
        repFileNamePattern,               // File name pattern
        reportsDir,       // Output folder
        "merged_data.csv",                 // Output file name
        inColDelimit,                    //input column delimiter
        outColDelimit,                  //output col delim
        keyFields,                      // Key fields for uniqueness (0-based index, e.g., first 3 columns)
        sortFields,                      // Sorting by first three fields (0-based index)
        mergerInfersHeader,              //true if the merger has to guess the header
        false,                           // Keep header from first file - false because no header in these files
                                         //plus this flag is not tested and it does not work well with mergerInfersHeader.
                                         //set it always to false
        csvMergeThreads                  // Use 4 threads
      );
    } */
    Logger::info("DRIVER", "Program END.");

    return 0;
}
