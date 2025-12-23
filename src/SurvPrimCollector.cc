#include "SurvPrimCollector.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include "SurvPrimCollector.hh"

#include <iostream>
#include <iomanip>
#include <cmath>
#include "Logger.hh"
#include "CustomCSVBuilder.hh"
//#include "G4Types.hh"

std::string survpCsvSeparator = std::getenv("CSS_SURVP_SUMM_CSV_SEP") ? std::getenv("CSS_SURVP_SUMM_CSV_SEP") : ";";
std::string survpCsvFilename = std::getenv("CSS_SURVP_SUMM_CSV_FNAME") ? std::getenv("CSS_SURVP_SUMM_CSV_FNAME") : "surv_prim.csv";
int survpCsvRngDigits = std::getenv("CSS_SURVP_SUMM_RNGDIGITS") ? std::stoi(std::getenv("CSS_SURVP_SUMM_RNGDIGITS")) : 3;

SurvPrimCollector::SurvPrimCollector(const int numOfBins, const G4String& outputDataDir): fNumBins(numOfBins), fOutputDataDir(outputDataDir) {
}

SurvPrimCollector::~SurvPrimCollector() {
}

void SurvPrimCollector::AddSurvivor(const std::string& layer, int evtId, const std::string& particleName, double energy) {
    std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("PRMSURV", "[" + layer + "][evt: " + std::to_string(evtId) + "] Primary particle PASSED: " + particleName + ", E=" + std::to_string(energy) + "MeV");
    auto& layer_rawEn = fRawEnergies[layer];
    auto& layer_energyRngs = fEnergyRanges[layer];
    layer_rawEn[particleName].push_back(energy);
    layer_energyRngs[particleName].update(energy);
}

void SurvPrimCollector::Finalize() {
    std::lock_guard<std::mutex> lock(fMutex);
    Logger::debug("SURVPP", "Finalizing the collectors");
    fHistograms.clear();
    for (const auto& layer_rawEn : fRawEnergies) { //loop on layers
      const std::string& layer = layer_rawEn.first;
      Logger::debug("SURVPP", "    Finalizing layer: " + layer);
      for (const auto& entry : layer_rawEn.second) { //loop on particles
        const std::string& partName = entry.first;
        Logger::debug("SURVPP", "        Finalizing layer's particle: " + partName);
        const std::vector<double>& energies = entry.second;
        const EnergyRange& range = fEnergyRanges[layer][partName];

        std::vector<int> bins(fNumBins, 0);
        double binWidth = (range.max - range.min) / fNumBins;

        for (double e : energies) {
            int binIndex = (binWidth > 0.0)
                ? static_cast<int>((e - range.min) / binWidth)
                : 0;
            if (binIndex >= fNumBins) binIndex = fNumBins - 1;
            bins[binIndex]++;
        }

        fHistograms[layer][partName] = std::move(bins);
      } //end loop on particles
    } //end loop on layers
    Logger::debug("SURVPP", "Collectors finalized.");
}

void SurvPrimCollector::PrintSummary() const {
    std::lock_guard<std::mutex> lock(fMutex);
    G4cout << "[SURVPP] === Passing Primary Particles: breakdown (num * MeV brackets) =========" << G4endl;

    std::vector<std::string> survpHeaders;
    std::stringstream ss("Layer,Particle,E_Min,E_Max,Count");
    std::string item;
    while (std::getline(ss, item, ',')) {
        survpHeaders.push_back(item);
    }
    //std::string fOuXtputDataDir = ".";
    G4String csvFullPath = fOutputDataDir + "/" + survpCsvFilename;
    CustomCSVBuilder csvHelp = CustomCSVBuilder(survpCsvSeparator, survpHeaders, csvFullPath);
    bool withHead = true;
    csvHelp.open(withHead);

    for (const auto& layer_histog : fHistograms) { // loop on layers
      const std::string& layer = layer_histog.first;
      G4cout << "[SURVPP] Layer: " << layer << G4endl;
      for (const auto& entry : layer_histog.second) {
        const std::string& partName = entry.first;
        const EnergyRange& range = fEnergyRanges.at(layer).at(partName);
        const std::vector<int>& bins = entry.second;

        double binWidth = (range.max - range.min) / fNumBins;

        G4cout << "[SURVPP]     Particle: " << partName << G4endl;
        for (int i = 0; i < fNumBins; ++i) {
            double eMin = range.min + i * binWidth;
            double eMax = eMin + binWidth;
            int curCount = bins[i];
            std::string eMinStr = (std::ostringstream() << std::fixed << std::setprecision(survpCsvRngDigits) << eMin).str();
            std::string eMaxStr = (std::ostringstream() << std::fixed << std::setprecision(survpCsvRngDigits) << eMax).str();
            //G4cout << "[SURVPP]        " << std::fixed << std::setprecision(3)
            //       << "[" << eMin << ", " << eMax << "): " << curCount << G4endl;
            G4cout << "[SURVPP]        " << "[" << eMinStr << ", " << eMaxStr << "): " << curCount << G4endl;
            csvHelp.addDataField(layer);
            csvHelp.addDataField(partName);
            csvHelp.addDataField(eMinStr);
            csvHelp.addDataField(eMaxStr);
            csvHelp.addDataField(curCount);
        }
      }
    }
    G4cout << "[SURVPP] ===========================================" << G4endl;
    csvHelp.close();
    Logger::info("SURVPP", "Survived primary particles summary saved to: " + csvFullPath);
}

void SurvPrimCollector::clear() {
    Logger::debug("SURVPP", "Clearing Survived primaries collector data structures");
    fHistograms.clear();
    fRawEnergies.clear();
    fEnergyRanges.clear();
}

