#pragma once

#include <map>
#include <vector>
#include <string>
#include <cfloat>
#include <mutex>
#include "G4String.hh"

class SurvPrimCollector {
private:

    struct EnergyRange {
        double min = DBL_MAX;
        double max = -DBL_MAX;

        void update(double value) {
            if (value < min) min = value;
            if (value > max) max = value;
        }
    };

    //struct ParticleInfo {
    //    //std::string& pName;
    //    int pCount = 0;
    //};

    mutable std::mutex fMutex;
    int fNumBins = 10;
    mutable std::string fOutputDataDir;

    // Temporarily collect raw energies before binning
    //std::map<std::string, std::vector<double>> fRawEnergies;
    //std::map<std::string, EnergyRange> fEnergyRanges;
    //Temporarily collect raw energies before binning
    //layer-keyed map of particle-keyed maps
    std::map<std::string,std::map<std::string, std::vector<double>>> fRawEnergies;
    std::map<std::string,std::map<std::string, EnergyRange>> fEnergyRanges;

    // Final histogram [layer -> particleName -> bin counts]
    std::map<std::string,std::map<std::string, std::vector<int>>> fHistograms;

public:
    SurvPrimCollector(const int numOfBins, const G4String& outputDataDir);
    virtual ~SurvPrimCollector();

    void clear();
    void AddSurvivor(const std::string& layer, int evtId, const std::string& particleName, double energy);
    void Finalize();
    void PrintSummary() const;
};

