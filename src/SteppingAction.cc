#include "SteppingAction.hh"
#include "ParticleDataManager.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4TouchableHandle.hh"
#include "G4SystemOfUnits.hh"
//#include "G4AnalysisManager.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "Logger.hh"
#include "Utils.hh"
#include "G4Types.hh"
//#include "CsvHelper.hh"

static const G4double joule_per_mev = 1.60218e-13; // J/MeV

SteppingAction::SteppingAction(StepDataWriter& stepDataWr, ParticleDataManager& pdm, WRResolver& wrRes, const bool saveStepData, const G4double chartZOffs, const bool stepDataCsvAllFields, DetectorConstruction& detConstr , const bool glbSummEnab, const bool thrSummEnab, const G4bool radiatON, CsvHelper& csvHelper, const bool csvIsG4Impl): fCsvIsG4Impl(csvIsG4Impl), fStepDataWr(stepDataWr), fcsvHelper(csvHelper), fWtneDoseOn(radiatON), fWrRes(wrRes), fSaveStepData(saveStepData), fChartZOffs(chartZOffs), fStepDataCsvAllFields(stepDataCsvAllFields), fDetConstr(detConstr), fPartDataMgr(pdm), fGlbSummEnab(glbSummEnab), fThrSummEnab(thrSummEnab) {
    fSummariesNeeded = (fGlbSummEnab || fThrSummEnab);
    fLayerNames = fDetConstr.getLayerNames();
    fLayers = fDetConstr.GetLayerConfigs();
    fOperCount = 0;
    if (fThrSummEnab) {
        for (const auto& layer : fLayerNames) {
            Logger::debug("G4ENGINE", "SteppingAction::constructor - Initializing fDataPerVolumeAndType[" + layer + "] map for thread-based summary");
            fDataPerVolumeAndType[layer];  // creates an empty inner map for this layer
        }
        Logger::debug("G4ENGINE", "SteppingAction::constructor - Initializing fDataPerVolumeAndType[<detector>] map for thread-based summary");
    }
    if (fGlbSummEnab) {
        pdm.initLayerDataMaps(fLayerNames);
    }

    for (const auto& layer : fLayerNames) {
        Logger::debug("G4ENGINE", "SteppingAction::constructor - Creating empty tracker for layer '" + layer + "' entrances");
        fEnteredLayer[layer]; // creates an empty inner map for this layer
    }
}

SteppingAction::~SteppingAction() {
}

void SteppingAction::UserSteppingAction(const G4Step* step) {
    fOperCount++;
    auto prePoint = step->GetPreStepPoint();
    auto postPoint = step->GetPostStepPoint();

    G4Track* track = step->GetTrack();
    G4int trackID = track->GetTrackID();
    G4int eventId = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

    auto preVol = prePoint->GetTouchableHandle()->GetVolume()->GetLogicalVolume();
    auto postVol = postPoint->GetTouchableHandle()->GetVolume() ?
                   postPoint->GetTouchableHandle()->GetVolume()->GetLogicalVolume() : nullptr;

    auto volumeName = preVol->GetName();
    auto particleName = track->GetDefinition()->GetParticleName();
    auto kineticEnergy = prePoint->GetKineticEnergy();

    G4double enDep = step->GetTotalEnergyDeposit();

    G4double mass = prePoint->GetPhysicalVolume()->GetLogicalVolume()->GetMass(); // [kg]
    G4double wtNormEquivDose = 0.0;
    //if (fWtneDoseOn) {
    //  if (mass > 0) {
    //      G4double wr = fWrRes.ResolveWR(particleName, kineticEnergy);
    //      G4double dose = enDep * joule_per_mev / mass; // in Gray
    //      wtNormEquivDose = dose * wr; // to get Sievert, it would be also * WT
    //  }
    //}

    auto volumeNamePost = (postVol ? postVol->GetName() : "None");
    G4double residualEnergy = postPoint->GetKineticEnergy();
    // Summary updates : Loop over known layers
    if (fSummariesNeeded) {
      //for (const auto& layXer : fLayerNames) {
      for (const auto& layerConfig : fLayers) {
        auto layer = layerConfig.name;
        G4bool cntAllLayerEntrances = layerConfig.countAllEntrances; //TBD depends on layer.., in detector was fallse .
        std::pair<G4int, G4int> particleKey;
        if (!cntAllLayerEntrances) {
          particleKey = {eventId, trackID};
        }
        auto& curLayer_fEntdLayer = fEnteredLayer[layer];
        G4bool addCondit = (cntAllLayerEntrances || !curLayer_fEntdLayer[particleKey]);
        auto& curLayer_Particle_Data = fDataPerVolumeAndType[layer][particleName];

        // Particle enters the layer
        if ((volumeName != layer) && (volumeNamePost == layer) && addCondit) {
            if (fThrSummEnab) {
                curLayer_Particle_Data.countIn++;
                curLayer_Particle_Data.energyIn += kineticEnergy;
            }
            if (fGlbSummEnab) {
                fPartDataMgr.trackParticleIn(kineticEnergy, layer, particleName);
                G4bool requiresPrimarySurvTracking = layerConfig.trackPrimPartSurv; //TBD depends on layer, and in detector was so
                //if we allow this in more than one layer, fPartDataMgr.trackSurvPrimary must be modified too,
                if (requiresPrimarySurvTracking && (trackID == 1)) { // only particles with trackId == 1 are primary!
                    fPartDataMgr.trackSurvPrimary(layer, eventId, particleName, kineticEnergy);
                }

            }
            if (!cntAllLayerEntrances) {
                //if the check is not required, no point in updating this map.
                curLayer_fEntdLayer[particleKey] = true;
            }
        }

        // Particle exits the layer
        if ((volumeName == layer) && (volumeNamePost != layer)) {
            G4bool requiresResidKEnergyUpEx = layerConfig.reqResidEnOnExit; //TBD depends on layer, was true only for detector
            G4double exitKinEnergy = kineticEnergy;
            if (requiresResidKEnergyUpEx) {
                exitKinEnergy = residualEnergy;
            }
            if (fThrSummEnab) {
                curLayer_Particle_Data.countOut++;
                curLayer_Particle_Data.energyOut += exitKinEnergy;
            }
            if (fGlbSummEnab) {
                fPartDataMgr.trackParticleOut(exitKinEnergy, layer, particleName);
            }
        }

        //all steps in the layer (enDep)
        if ((volumeName == layer) && (enDep > 0)) {
            G4bool evalRadReqd = fWtneDoseOn && layerConfig.evalRad;
            if (evalRadReqd) {
                if (mass > 0) {
                    G4double wr = fWrRes.ResolveWR(particleName, kineticEnergy);
                    G4double dose = enDep * joule_per_mev / mass; // in Gray
                    wtNormEquivDose = dose * wr; // to get Sievert, it would be also * WT
                }
            }
            if (fThrSummEnab) {
                curLayer_Particle_Data.energyDep += enDep;
                curLayer_Particle_Data.wtNormEqDose += wtNormEquivDose;
            }
            if (fGlbSummEnab) {
                fPartDataMgr.trackEnergyDeps(enDep, wtNormEquivDose, layer, particleName);
            }
        }
      } // end loop over layers
    } // end if summaries needed

    ///analysis
    if (fSaveStepData) {
        auto stepLength = step->GetStepLength();
        const G4ThreeVector& prePointPos = prePoint->GetPosition();
        G4double zPos = prePointPos.z();
        G4int stepNumber = track->GetCurrentStepNumber();
        G4int threadId = G4Threading::G4GetThreadId();

        unsigned fIdx = 0;
        if (fCsvIsG4Impl) {
        fcsvHelper.addDataField(fIdx++, eventId);
        fcsvHelper.addDataField(fIdx++, trackID);
        fcsvHelper.addDataField(fIdx++, stepNumber);
        fcsvHelper.addDataField(fIdx++, threadId);
        fcsvHelper.addDataField(fIdx++, zPos + fChartZOffs);
        fcsvHelper.addDataField(fIdx++, stepLength);
        fcsvHelper.addDataField(fIdx++, particleName);
        fcsvHelper.addDataField(fIdx++, volumeName);
        fcsvHelper.addDataField(fIdx++, volumeNamePost);
        fcsvHelper.addDataField(fIdx++, kineticEnergy);
        fcsvHelper.addDataField(fIdx++, residualEnergy);
        } else {
        fStepDataWr.addDataField(eventId);
        fStepDataWr.addDataField(trackID);
        fStepDataWr.addDataField(stepNumber);
        fStepDataWr.addDataField(threadId);
        fStepDataWr.addDataField(zPos + fChartZOffs);
        fStepDataWr.addDataField(stepLength);
        fStepDataWr.addDataField(particleName);
        fStepDataWr.addDataField(volumeName);
        fStepDataWr.addDataField(volumeNamePost);
        fStepDataWr.addDataField(kineticEnergy);
        fStepDataWr.addDataField(residualEnergy);
        }
        if (fStepDataCsvAllFields) {
            G4double xPos = prePointPos.x();
            G4double yPos = prePointPos.y();
            G4ThreeVector momDir = prePoint->GetMomentumDirection();
            G4int trackParentID = track->GetParentID();
            G4double momDirX = momDir.x();
            G4double momDirY = momDir.y();
            G4double momDirZ = momDir.z();
            const G4VProcess* process = prePoint->GetProcessDefinedStep();
            G4String processName = process ? process->GetProcessName() : "none";
            if (fCsvIsG4Impl) {
            fcsvHelper.addDataField(fIdx++, xPos);
            fcsvHelper.addDataField(fIdx++, yPos);
            fcsvHelper.addDataField(fIdx++, momDirX);
            fcsvHelper.addDataField(fIdx++, momDirY);
            fcsvHelper.addDataField(fIdx++, momDirZ);
            fcsvHelper.addDataField(fIdx++, enDep);
            fcsvHelper.addDataField(fIdx++, trackParentID);
            fcsvHelper.addDataField(fIdx++, mass);
            fcsvHelper.addDataField(fIdx++, processName);
            } else {
            fStepDataWr.addDataField(xPos);
            fStepDataWr.addDataField(yPos);
            fStepDataWr.addDataField(momDirX);
            fStepDataWr.addDataField(momDirY);
            fStepDataWr.addDataField(momDirZ);
            fStepDataWr.addDataField(enDep);
            fStepDataWr.addDataField(trackParentID);
            fStepDataWr.addDataField(mass);
            fStepDataWr.addDataField(processName);
            }
            if (fWtneDoseOn) {
                if (fCsvIsG4Impl) {
                fcsvHelper.addDataField(fIdx++, wtNormEquivDose);
                } else {
                fStepDataWr.addDataField(wtNormEquivDose);
                }
            }
        }
        if (fCsvIsG4Impl) {
        fcsvHelper.addRecord();
        }
    }
}

void SteppingAction::EndOfRunSummary() const {
    Logger::info("G4ENGINE", "Thread activity: " + std::to_string(fOperCount) + " steps processed.");
    if (fThrSummEnab) {
        std::string g4ThreadLabel = Utils::formatThreadLabel(G4Threading::G4GetThreadId());
        std::ostringstream eorSummary;
        eorSummary << "\n=== [G4Thread: " << g4ThreadLabel << "] Run ended. Summary by Layer and Particle (scope: worker thread) ===\n";
        int layerIdx = 0;
        for (const auto& volumeName : fLayerNames) {
            layerIdx++;
            auto it = fDataPerVolumeAndType.find(volumeName);
            if (it == fDataPerVolumeAndType.end()) continue;  // Skip if not present

            const auto& particleMap = it->second;

            std::string layerLabel = Utils::formatLayerLabel(layerIdx);
            eorSummary << "Layer (" << layerLabel << "): " << volumeName << "\n";
            for (const auto& particleEntry : particleMap) {
                const auto& particleName = particleEntry.first;
                const auto& data = particleEntry.second;

                eorSummary << "    " << g4ThreadLabel << " | " << layerLabel << " | Particle: " << particleName
                    << " | In: " << data.countIn
                    << " | Out: " << data.countOut
                    << " | E_in: " << data.energyIn / MeV << " MeV"
                    << " | E_out: " << data.energyOut / MeV << " MeV"
                    << " | E_dep: " << data.energyDep / MeV << " MeV";
                if (fWtneDoseOn) {
                    eorSummary << " | WTNEDose: " << data.wtNormEqDose << " Sv/WT";
                }
                eorSummary << "\n"; //<< G4endl;
            } //end loop particles
        } //end loop layers

        eorSummary << "======================================\n";
        G4cout << eorSummary.str() << G4endl;
    } else {  //end if report by thread required
        Logger::debug("G4ENGINE", "Thread-based end of run summary: not available (disabled by configuration choice)");
    }
    for (auto& clayer : fLayerNames) {
        //we could surgically clear only the ones for which cntAllDetectEntrances was false, but not worth, TBD
        Logger::debug("G4ENGINE", "Clearing thread summary's layer '" + clayer + "'  entrances map");
        auto& curLayer_fEntdLayer = fEnteredLayer.at(clayer);
        //auto& curLayer_fEntdLayer = fEnteredLayer[clayer];
        curLayer_fEntdLayer.clear();
    }
    Logger::debug("G4ENGINE", "Clearing thread summary's data map");
    fDataPerVolumeAndType.clear();
}
