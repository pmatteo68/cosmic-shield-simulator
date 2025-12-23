# CSS - Cosmic Shield Simulator

## Software metadata

|Attribute|Value|
|:-|:-|
|Version|5.1.5|
|Date|Aug. 8, 2025|
|Software architecture, design, development, maintenance|[Matteo P.](https://www.linkedin.com/in/mattprmlv/) (Software Architect, Electronics Engineer)|
|Functional requirements ownership & roadmap|[Filippo M. Soccorsi](https://www.linkedin.com/in/fmsoccorsi) (Astrophysics researcher at "La Sapienza" University of Rome, Italy)|

## Major contributions
* [Filippo M. Soccorsi](https://www.linkedin.com/in/fmsoccorsi) (Astrophysics researcher at "La Sapienza" University of Rome, Italy) is not only the functional owner of this package, as he also provided substantial contribution in the following areas, for which I am deeply grateful:
  * First prototype development
  * Detailed output results set definition
  * Testing
  * Issue troubleshooting
  * Sharing thoughts throughout the entire lifetime of the simulator, being constant inspiration for enhancement and improvement

## Abstract
Cosmic Shield Simulator (CSS) is a C++ software framework based on [Geant4](https://geant4.org) for analyzing the interaction of multi-particle beams with multi-material, multi-layer planar shielding structures.

## Simulation scenario
A beam propagates over Z axis, perpendicularly meeting a planar structure made up by a series of layers:

```
                       ┌───────┐┌───────┐┌───────┐
                       │       ||       ||       |
                       │       ||       ||       |
                       │       ||       ||       |
 ORIGIN →→→→→→→→→→→→→→ │Layer 1││Layer 2││Layer 3│ ...
                       │       ||       ||       |
                       │       ||       ||       |
                       │       ||       ||       |
                       └───────┘└───────┘└───────┘

```
The framework allows you to configure:

* The [physics list](https://geant4.web.cern.ch/documentation/dev/plg_html/PhysicsListGuide/physicslistguide.html) to be utilized

* The beam composition (particle types - such as electrons, protons, neutrons, alpha particles, photons, heavy ions and more - and amount)

* The geometry (number of layers, along with their thickness, and the materials they are made of)

* The rules for the WR calculations ([radiation weighting factor](https://radiopaedia.org/articles/radiation-weighting-factor)) which matters if you are interested in evaluating the radiation impact)

## Prerequisites

* [Geant4](https://geant4.org) v. 11.3.1 or sup., and its related [dependencies](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/gettingstarted.html)

### Tested Combinations

We have successfully built and executed this application on the following HW/SW combinations.

#### AWS - RHEL 9.6
The following combinations were tested on a 2 x Intel(R) Xeon(R) Platinum 8259CL CPU @ 2.50GHz, 4 Gb RAM, 100 Gb SDD HD (AWS AMI ID: ami-0179b788832c41bfb, amazon/RHEL-9.5.0_HVM-20250313-x86_64-0-Hourly2-GP3).

|Geant4 version|Geant4 build properties
|:-|:-|
|11.3.1|none (i.e. defaults apply)|
|11.3.2|GEANT4_INSTALL_EXAMPLES=ON<br> GEANT4_USE_GDML=ON<br> GEANT4_BUILD_MULTITHREADED=ON<br> GEANT4_USE_RAYTRACER_X11=ON<br> GEANT4_INSTALL_DATA=ON<br> GEANT4_BUILD_STORE_TRAJECTORY=ON<br> CMAKE_BUILD_TYPE=Release GEANT4_USE_QT=OFF<br> GEANT4_USE_OPENGL=ON<br> GEANT4_USE_OPENGL_X11=ON<br> GEANT4_USE_XM=OFF|
|11.4.0|GEANT4_INSTALL_EXAMPLES=ON<br> GEANT4_USE_GDML=ON<br> GEANT4_BUILD_MULTITHREADED=ON<br> GEANT4_USE_RAYTRACER_X11=ON<br> GEANT4_INSTALL_DATA=ON<br> GEANT4_BUILD_STORE_TRAJECTORY=ON<br> CMAKE_BUILD_TYPE=Release GEANT4_USE_QT=OFF<br> GEANT4_USE_OPENGL=ON<br> GEANT4_USE_OPENGL_X11=ON<br> GEANT4_USE_XM=OFF|

## Installation

1. Deflate the archive you received, or clone the GIT repository if any, etc.

2a. Configure the main scripts: edit the `scripts/bin/g4env*.sh` (and any other, in the same directory) so that
    to match your settings as per XERCES_VER, G4_VER, G4_NPROC (this one should match the output of the `nproc` command on your machine).

2b. Review the project scripts under `projects/cosmic-shield-sim/v<version>-<yyMMdd>/proj`. Specifically, check which one, among the ones you edited at the step above, is invoked by `build-cssim.sh` and `bin/xrun-cssim-single.sh` (it's visible in their first lines), and make sure all is compliant to your expectation. Additionally, you may want to check the `bin/env-cssim-tr.sh`, to verify the location of the testrunner's data home (i.e. where the testrunner will save data).

3. Build.

```
cd <project's proj directory, i.e. where the scripts are>
./build-cssim.sh
```

## Usage

PREMISE: ALL the scripts mentioned below have to be launched from a command shell in the same directory containing them.

* To run one single simulation (in batch/unattended mode, or with the UI (*)), configure the run first (mainly: all the JSON files under `config` directory, the editable variables in the `run-cssim.sh` script, the beam size and number of threads in the `macros/run.mac` Geant4 macro script), then launch:

```
./run-cssim.sh {"batch"|"ui"}
```

* There is also a utility to run multiple simulations in unattended mode, ref. section "TESTRUNNER" below.

* There is also a utility to dump the entire materials database to a file, ref. section "Geant4 Materials Database Dumping Utility" below.


### Postprocessing utilities
* CSV MERGER. If the step data were saved and if multiple threads were configured, the `css-merger.sh` utility will merge the per-thread CSV files into a single one.

* CSV SORTER. If the step data were saved and if multiple threads were configured, the `css-sorter.sh` utility will sort the step data single CSV file by the fields you choose.

### Geant4 Materials Database Dumping Utility
* You can launch the following script to dump the materials database: `./dump-g4-materials-db.sh`. The target files - both TXT (names list only) and JSON (names and densities) formats will be generated - can be specified in the script itself. Note that both these formats can be of specific interest 'per se', but also they come handy if you are utilizing the Optimizer (a separate project which launches many iterations of this simulator in the hope of determining the geometry of an optimal shield). This script also generates separate files with names (TXT) or names/densities (JSON) for the custom materials definitions. With all these artifacts, the user will be put in position to easily configure the optimizer.

### TESTRUNNER
Utility to launch multiple tests in sequence, in an unattended fashion (and you can even quit the session while it's running, as it is executed in background). 

```
./css-testrunner.sh {<dir. or archive>}
```

The testrunner utility accepts, as an argument, the test directory path OR an archive (tar, zip, gzip, tar.gz, tgz) containing such directory. The directory needs to have this structure in order to be successfully processed:

```
<whichever directory name>/
 |
 .--- common/ (contains COMMON configuration json and mac files)
 |  
 .--- test-1
 |      |
 |      .-----in (contains test1-specific configuration json and mac files)
 |
(etc, test-2, test-3, ...)
```

If a json or a mac file is found BOTH in the `common` and in a `test-<J>/in` folder, the test-specific copy takes precedence.

The output will be created, for each test, under the `test-<J>/out` directory.

## Technical notes

### Memory analysis on RHEL
* In case you need to analyze memory usage, on RHEL Valgrind has proven being a good option. Suitable command lines have been: `valgrind --leak-check=summary --track-origins=no ./${CSS_EXE} \ ...` (in the script, under `bin` subdirectory, which is responsible for ultimately launching the program. Heavier options like ` ... --leak-check=full --track-origins=yes ...` could hit the performance too severily. Use with caution.

### Aggregates data outputs
The following aggregate CSV files are generated at the end of the simulation.

* Layer energy summary: energies in-out into and out of each layer, particles in-out into and out of each layer, and similar quantities.

* Beam composition breakdown. The particles are selected randomly, given the weight assigned to each particle (ref. flux field in the beam configuration file). A CSV is generated in the end, showing the ACTUAL beam composition that had been utilized. Note that this file will show also ALIASES-NAMES for custom particles (where, for custom particles, the alias is the one in the configuration file where they are defined, and the name is the one assigned to it by Geant4 framework upon creation; this is useful as it is NOT possible to have Geant4 creating them with a predefined name, so checking the mapping is useful for analysis, and also for WR definition etc.).

* Survivor primary particles breakdown. This CSV shows, out of each layer for which this feature has been activated (ref. geometry configuration file), how many of the primary particles made it through.

### Step data CSV trace
Step data CSV trace files are generated one per each thread, they can be huge files, therefore a flag to turn this feature on/off is provided. A snippet follows, showing headers and some values.

|EventId|TrackId|StepNbr|ThreadId|zPos|StepLen|Particle|VolPre|VolPost|kineticEnergy|residualEnergy|xPos|yPos|momDirX|momDirY|momDirZ|depEnergy|TrackParentId|mass|Process|WTNEDose|
|--------:|--------:|--------:|---------:|---------:|-----------:|:--------:|:---------:|:--------:|---------------:|----------------:|---------:|-----:|--------:|--------:|--------:|----------:|---------------:|-------------:|:---------------:|------------------:|
|18|1|1|0|-1000|10|gamma|World|AlLayer1|10|10|0|0|0|0|1|0|0|1.15438e+28|none|0|
|18|1|2|0|-990|50|gamma|AlLayer1|PolyLayer|10|10|0|0|0|0|1|0|0|8.42292e+26|Transportation|0|
|18|1|3|0|-940|50|gamma|PolyLayer|WLayer|10|10|0|0|0|0|1|0|0|2.93351e+26|Transportation|0|
|18|1|4|0|-890|10.3564|gamma|WLayer|WLayer|10|0|0|0|0|0|1|0|0|6.02306e+27|Transportation|0|
|18|4|1|0|-879.644|4.64479e-07|W184|WLayer|WLayer|3.30008e-07|0|0|0|-0.14526|0.84911|0.50785|3.30008e-07|1|6.02306e+27|none|4.38924e-47|

A few important points.

* `EventId` and `TrackId` fully identify a particle. The `EventId` is a unique number assigned to each primary event (i.e., a simulation of one incoming particle or group). The `TrackId` is a unique ID for each particle track within an event. Primary particle get TrackId = 1, secondaries get >1

* The generic particle's motion within the structure is modeled by a series of STEPS, therefore the (`EventId`, `TrackId`, `StepNbr`) triplet is a primary key for the CSV. First step is usually when particle enters the geometry.

* Keep in mind that, when operating with multi-threading, each thread is assigned an ENTIRE event (and its entire set of particles).

Ideas for post-processing:

* Group by eventID. Plot total energy deposition or range per event. Compare event-wise behavior or filtering anomalies.

* Group by trackID. Track the energy loss of a single particle. Plot kinetic/residual energy vs. step number.

* Group by particleName. Compare energy loss profiles across particle types. Histogram of number of steps per particle type.

* Group by (eventID, trackID). Reconstruct complete path for each particle. Visualize energy deposition along z for each track.

* Group by stepNumber. Average energy values across all particles at each step index.

* Bin by depth (zPos)
Heatmaps / histograms of energy at different depths. Mean/residual energy vs. depth per particle type.

### Notes about geometry
The geometry configuration file allows, at the level of each layer, to prescribe that each inbound particle has to be accounted for only once. This can be useful for the last layer when it's supposed to be a  detector (backscattering would not need to be accounted for in the detector case).

### Geometry Evaluations
New in [4.3.2], a csv file is generated showing, for each layer, some calculated data.
* Density (g/cm3)
* Normalized weight (Kg/m2).

Note that you can also EXCLUDE layers from the density/weight calculations (ref. proper parameter in the editable section of the launch scripts, this is useful for example if you want to exclude detector layers if any).

### Notes about Logging
There is a wrapper class with utility methods, based on G4 logging library. Note it could be easily transformed in standard-only based, i.e. not dependent on G4, just by rapid modifications (final `cout`).

## GUI (Graphic User Interface)
Introduced in version 3.7.5, the OpenGL UI still seems not very usable. Ref. Open Points.

## Multithreading
Threads work unevenly: it is systemic, and normal, and it is more and more evident on small runs. Interesting lectures over multithreading follow:

* [Geant4 - Multithreading](https://geant4.web.cern.ch/documentation/pipelines/master/bftd_html/ForToolkitDeveloper/OOAnalysisDesign/Multithreading/mt.html)
* [Geant4 - How to migrate to multithreading](https://indico.esa.int/event/50/contributions/2583/attachments/2092/2440/HowToMigrateToMultiThread.pdf)

## Global KPIs calculation
New in [4.3.3], a CSV is created, containing global KPIs for the structure. Currently available metrics are:
* EnergyEfficiency (defined as 1 - Eout/Ebeam, where Eout is the energy exiting from the penultimate layer. Note that `CSS_KPI_LAYERS_TO_DISCARD` and `CSS_KPI_EVAL_DIRECTION` environment variables, in the `bin/xrun-cssim-single.sh` script, can be used to customize the calculation). The defaults are good for the scenario in which we refer to the layer before the last one (which typically is the detector), using `Eout` for the calculation.
* [4.3.4] ProtectionEfficiency, defined similarly to EnergyEfficiency, with the addition of the WR coeeficients being evaluated and accounted for.
* Global Normalized Weight (of the entire shield)
* [4.3.5] Global shield thickness

## Known issues
* Sometimes the program exits without having carried out the simulation. NOT NECESSARILY an issue. With certain configurations, it may happen that, by design, the simulation won't be performed. READ THE LOGS ACCURATELY AS TO IN WHICH CIRCUMSTANCE DID THIS HAPPEN, AND WHY.

## Open Points

* We should make easily possible to have configs and macros in an external directory. It is possible by tweaking the scripts, but it should be made easier.

* Sometimes, the very first run, on a fresh new AWS machine, hangs. Then, the problem disappears with any subsequent run.

* Any measure, in literature, similar to the radiation, but more oriented to malfunction for electronic circuitry?

* The CsvHelper (G4-based csv management for step data csv) and the StepDataWriter (custom) may be merged in a single class which wraps both details.

* Having test runner sending an email in the end?

* GUI not much usable: it takes about 10 mins to come up even for simple simulations, also many weird messages come up on the console. Also, the PDF returned by the UI are all black pages ??

* Stricter input checks maybe needed? (enforce custom materials' components fractions sum to ONE? Need to check if this is already enforced. and other similar things, given that in many cases of inconsistent configurations there are un-friendly core dumps ...)

* Performance: separate `mutex` objects in the PartDataManager ? They can even be at the level of the individual KEYs.

* Introducing the possibility of some coarse grain in the step data CSV? eg mod%4 etc

* `sh` sorter utility: ascertain that it can be flexible as to separators, with an input of 0,1,2 (it should assume no header, and not place it in the output either. Also, if the sortBy is empty string, no processing should take place).

* Local testing to complete: ensuring merging/sorting utils would work also with 1 thread.

* A utility to check primary key in a CSV should be provided.

* Histograms generation can be added? There are some native Geant4 call for this ([here](https://geant4-forum.web.cern.ch/t/defining-writing-a-histogram/1877), and also many other posts). Also the example in install/share/Geant4/examples/basic/B4/B4a/src shows some of that.

* `analyzeSummary-withEnergy.sh` does not manage the eDep, starting from 3.7.3, and does not manage the parent id starting from 3.7.6, and subsequently added csv fields (also shows: line 77: [[: 0.00806047: syntax error: invalid arithmetic operator (error token is ".00806047")). HOWEVER, not sure additional analysis will be carried on about this, since the `analyzeSummary-withEnergy.sh` and `analyzeSummary.sh` will be suppressed soon (they only server the purpose of supporting solid and rapid testing when the PartDataManager singleton was introduced, so to have the global summary instead of the per-thread summaries).

* Log can be evoluted, for example 'a la Log4J' with different appenders, rolling files etc? EG spdlog, log4cpp, Boost.Log, easylogging++, glog (Google Logging), Pantheios, plog and more. Or maybe there is a "Geant4 way" to do it?

* In RunAction the analysis manager type always comes out as `undefined`. Not a problem.

* It seems that maxStep is not working as expected. But, not sure it's a problem. Maybe it is BY DESIGN, conceived just like a hint. Ref. [here](https://geant4-forum.web.cern.ch/t/maximum-step-length/2178/3).

* /geometry/test/run can be used, in a macro, to test. Need to investigate about this.

* Productivity utility for team members: prepare4Upload or something, also more aliases?

* Not sure how to identify the master thread. Tried all the possible, including `rm->GetRunManagerType() == G4RunManager::RMType::masterRM`, but it does not work.   Ref. also attempts in `RunAction.cc`. Should I import `globals.hh` or `G4RunManager.hh` for this? Relevant example on this is in Geant4 example B4a (install/share/Geant4/examples/basic/B4/B4a/src)

* Per-thread step data CSV do not seem allowing customization of the separator. This is addressed by the `sh` merging and sorting utilities, but still a Geant4-native way to control the separators and the format in general would be appreciated.

* Suppressing physics list initialization log on demand? For now, seems possible only for explicitly initialized physics lists. Tried `/run/verbose 0`, `/event/verbose 0`, `/tracking/verbose 0`, `/geometry/navigator/verbose 0`; in the macro, didn't work. Tried `export G4VERBOSE=NONE`, didn't work. Also tried `physicLists.setVerbose(0)`, didn't work. Addressed in 3.7.0 with a TRICK. if it is `QGSP_BIC_AllHP` (or a few others), then I load it statically, and in that case the constructor accepts - and HONORS - a verbosity setting. But, dynamically, it does not (the setVerbosity method seemingly does not work). This is a good trick as long as the physics list is stable in our research, and won't change.

* The run script should also log dependencies versions, eg `geant4-config --version`, `geant4-config --libs`, et al.

* CMake file: build happens without copying macros and config, but also install INSTALL should be planned for, and macros and config should be shipped there. NOTE: the CMake file has already, commented, what needed.

* Logging number of threads upfront? Seems difficult. Settings those in the macro works, but then all the things I tried would log "2" regardless of the fact that there are a different number.

* Run script flags to be documented better.

* To consider whether IONS (eg Dy159[309.593], Cs128[187.570X] etc., eventually should be groupable or not, from the standpoint of the aggregates evaluations.

* WR resolution on/off to be made configurable in the RUN and TESTRUNNER run scripts?

* WR resolution (since 3.7.8): can be impacting the performance, we may come up with a lightweight satisfactory approximation? For now, the in-line calculation is toggable.

* Maybe to resolve the limitation forcing us to have world origin in 0-0-0.
The world position is 0.0, 0.0, 0.0 by default if not specified. NOTE: although this is configurable software-wise, Geant4 DOES NOT SUPPORT positions other than 0-0-0 for the world object. The application exposes this parameter as configurable just in case things change in future. And the workaround (creating an innerworld) would introduce complexity, and we won't do it, at least for the moment. Exception is:
```
*** G4Exception : GeomNav0002
      issued by : G4Navigator::SetWorldVolume()
Volume must be centered on the origin.
```

* In the survivor particles class (`SurvPrimCollector.cc`) there is minor inconsistency, with both `G4int` and `int` utilized. Let's decide on just one? This does not affect only this class indeed, a thorough review over the entire codebase should be carried out.

* WR Rules eventually configurable via `run.mac` too? With all the optional parameters in there, it's a lot of effort, and not sure it's worth it.

* It'd be better to have the Geant4 materials database dumping in another C++ executable. Also, direct merge with custom materials would be nice for this tool.

## Closed Points

* [250801-5.1.0] "malloc(): unsorted double linked list corrupted" program crash happening with some configurations. Reported by F. M. Soccorsi on Jul. 30, 2025.
  WORKAROUND: in `bin/xrun-cssim-single.sh`, set the CSS_GLOBAL_PART_SUMMARY to OFF. This way you give up the particles summary (and this is why we call it a workaround and not a solution),
  but, at least, program won't crash, and KPIs will be generated.
  RCA: ParticleDataManager no longer relies on the reference to the DetectorConstructor, which was deleted/destroyed by Geant4 at the end of the simulation, PRIOR to ParticleDataManager accessing it for calculations, which ultimately caused the crash. Noticeably, the corruption would not show up immediately, but only when the CustomCsvBuilder was built.

* Not often, sometimes this appeared:
```
Graphics systems deleted.
Visualization Manager deleting...
double free or corruption (out)
```
It seemed strange. We needed to reproduce it systematically, and to resolve it. At one point, around version 3.8.x, the problem stopped showing.

* Not often, sometimes this appeared:
```
G4WT1 > [250503-04:24:21:838][140673533605440][1][DEBUG][G4ENGINE] SteppingAction::constructor - Creating empty tracker for detector entrances
terminate called after throwing an instance of 'std::bad_alloc'
  what():  std::bad_alloc
G4WT0 > [250503-03:36:36:098][139779319113280][0][DEBUG][G4ENGINE] SteppingAction::constructor - Creating empty tracker for detector entrances
double free or corruption (!prev)
```
It seemed strange. We needed to reproduce it systematically, and to resolve it. At one point, around version 3.8.x, the problem stopped showing.

* True that nproc and num threads should match? That is only for the BUILD (`make -jN` - N should match the output of `nproc` command on the machine).

* Particles like `Dy159[177.614]` are ions. We could transform such strings in `Dy159_177.614` (less problems in parsing). Done in 3.7.3 also, which also covers patterns like `Cs128[187.570X]`. Note, the number is the excitation energy, and the trailing X is there if the ion is not listed in Geant4 maps.

## Obviated requirements
Just to keep track of past - and closed - analyses, we list here a few requirements that were first considered, then obviated.

### Special inbound counting strategy (F. M. Soccorsi, May 6, 2025, obviated on May 19 after thorough analysis).
In the case of a layer configured as Detector (i.e., that layer's stratum set for special counting), for each particle, counting and recording of parameters must occur only if the following three conditions are met:
The particle was not generated in the Detector AND
The particle enters the 'Detector' volume coming from an adjacent volume that precedes it along the direction of the z-axis. AND
After this entry, it does NOT re-enter the immediately preceding volume.
At the moment of the last and definitive entry of the particle into the detector, the parameters specified in the log or in the aggregated CSV must be recorded.

In case this requirement is revamped, there is a partial implementation in `v3.8.6-WIP-aborted250519`. Also, a few useful notes for the step status `G4StepPoint::GetStepStatus()`.
In `UserSteppingAction(const G4Step* step)`, post-step point state can be gathered with:
```
G4StepPoint* postPoint = step->GetPostStepPoint();
G4StepStatus status = postPoint->GetStepStatus();
```
Possible values are in the `enum` contained in `source/track/include/G4StepStatus.hh.status`, eg:

```
fWorldBoundary → track is leaving the world volume
fGeomBoundary → track is entering a new volume (also from layer to layer)
fPostStepDoItProc → the point has been created by a physical process (ionization, ecc.)
```

### Inline CSVMERGER

* Inline CSVMERGER class momentarily OUT of the codebase, last version it was included was 3.7.6. It basically performs bad, and also with weird issues (example: program hangs, or also zombi instance of the program would come up, consuming 200% CPU), upon high volumes. The separated sh utility works way better, so I don't believe there will be future attempts of re-introducing this feature in the C++ code. This item is mentioned only for the record.

* Inline CSVMERGER. Although not in use, we report that keepHeader and infer header are in conflict. Not well handled. keepHeader never really tested by the way. Unless solidified have ALWAYS keepHeader=false.

## Other GEANT4 references
* [Recent Developments in Geant4](https://www.sciencedirect.com/science/article/pii/S0168900216306957), J. Allison et al., Nucl. Instrum. Meth. A 835 (2016) 186-225
* [Geant4 Developments and Applications](https://ieeexplore.ieee.org/xpls/abs_all.jsp?isnumber=33833&arnumber=1610988&count=33&index=7), J. Allison et al., IEEE Trans. Nucl. Sci. 53 (2006) 270-278
* [Geant4 - A Simulation Toolkit](https://www.sciencedirect.com/science/article/pii/S0168900203013688), S. Agostinelli et al., Nucl. Instrum. Meth. A 506 (2003) 250-303

## Change Log

### [5.1.5] - 2025-08-08 (M. P.)

#### Added
* Added a utility to dump the materials database: `./dump-g4-materials-db.sh`. This is a helper for an easier utilization of the optimizer (a separate project for finding the optimal shield). Ref. section "Geant4 Materials Database Dumping Utility".
* Compiles also against Geant4 v. 11.4.0

### [5.1.4] - 2025-08-07 (M. P.)

#### Fixed
* The custom CSV builder had a loss of precision for the `double` fields. This has been fixed.

### [5.1.3] - 2025-08-05 (M. P.)

#### Added
* In the docs folder, we added the full materials list for Geant4 v11.3.2 (the inbuilt ones, i.e. the ones in the NIST database).

#### Changed
* Logging refinements

### [5.1.2] - 2025-08-04 (M. P.)

#### Added
* Now the optimizator will have the possibility to override the flag for the activation of the creation of the step data CSV files.
* Notes, in this README, about memory analysis with Valgrind.

#### Changed
* Some internal optimizations in the scripts.

### [5.1.1] - 2025-08-04 (M. P.)

#### Added
* Added a few notes in this README, about custom particles ALIASES

#### Fixed
* [250801-5.1.0] "malloc()..." issue and crash: fixed (ParticleDataManager no longer relies on the reference to the DetectorConstructor, which was deleted/destroyed by Geant4 at the end of the simulation, PRIOR to ParticleDataManager accessing it for calculations, which ultimately caused the crash).

### [5.1.0] - 2025-08-01 (M. P.)

#### Added
* Shell scripts modified to make the simulator compatible with the optimization engine.
* IMPORTANT: Documentation about an existing issue and related workaround, ref. open points ("malloc(): unsorted double linked list corrupted")
* NOTE: C++ part of the project is UNTOUCHED in this transition to 5.1.0

### [4.3.5] - 2025-07-28 (M. P.)

#### Added
* The KPIs csv file now contains also the global shield thickness.

#### Changed
* Now, for homogeneity purposes, the global geometry summary csv file does not contain anymore the 'ALL' row.
* Logging improved

### [4.3.4] - 2025-07-25 (M. P.)

#### Added
* Protection efficiency has been added to the global KPIs, as per requirements shared on Jul. 23, 2025, by F. M. Soccorsi.
* Also the global normalized weight, despite already present in the global geometry summary, is mentioned in the global KPIs output, for better/easier reading.
* Now the global geometry summary contains a few fields more (energy in/out and counts in/out - per layer).

### [4.3.3] - 2025-07-24 (M. P.)

#### Added
* Now a Global KPIs results CSV file is created, in addition, as per requirements shared on Jul. 23, 2025, by F. M. Soccorsi. Metrics in it: EnergyEfficiency (defined as 1 - Eout/Ebeam, where Eout is the energy exiting from the penultimate layer)

### [4.3.2] - 2025-06-30 (M. P.)

#### Added
* Now also a summary over the geometry is being generated (in the logs and as a new csv file). For each layer, density will be visible. Also, the normalized weight is displayed. Ref. section above about Geometry Evaluations for details about the units of measure.

### [4.3.1] - 2025-06-02 (M. P.)

#### Fixed
* Now the `data` and `step` constants (Geant4 stepdata csv base file name and tuple name) are not hardcoded anymore, and their configuration has been properly centralized and propagated.

### [4.3.0] - 2025-05-31 (M. P.)

#### Added
* Now the step data csv can be also generated in a custom way, i.e. out of the Geant4 framework. Ref. new flag in the driver scripts (`run-cssim.sh`, `css-testrunner.sh`). Note that, if you so choose, performance may be impacted, as the multiple threads now have a contention against the single file being written. But, on the flip side, no merging will be needed at the end.

### [4.2.0] - 2025-05-28 (M. P.)

#### Added
* Now the geometry configuration allows to decide whether we want to evaluate radiations at the level of the individual layer. Ref. new field `evalRadiations`. When not specified, default is false. IMPORTANT: in the run scripts (`run-cssim.sh`, `css-testrunner.sh`) the flag for turning ON/OFF the evaluation of radiations acts as a main switch, superseding any setting in the geometry configuration file if set to OFF. In other words: if the main switch is OFF, no evaluation of radiations takes place, period. If the main switch is ON, then the settings in the geometry configuration file apply.
* This new layer-level flag can also be set via macro, ref. example provided (commented in `run.mac`).

#### Changed
* Now the flag for turning ON/OFF the evaluation of the radiation is in the "surfacing" scripts (`run-cssim.sh`, `css-testrunner.sh`). Note, also, that this flag acts as a main switch: if it's OFF, no evaluation of radiations will take place, regardless of what is defined in any macro or configuration file.
* Now the merging utility prints a timestamp during its operations.
* Now the test runner main scripts has a flag allowing to delete (or not) the raw step data CSV files in case they were created, and in case the merging was enabled.
* Now the initialization log shows how many WR rules were loaded.


#### Fixed
*

### [4.1.1] - 2025-05-21 (M. P.)

#### Added
* Now the user can specify custom materials' elements and materials also with MAC file. Ref. commented examples in run.mac.
* Now the user can execute MULTIPLE tests in sequence, in background. Ref. section TESTRUNNER

#### Changed
* Now the program can be launched with: `./run-cssim.sh batch` for batch mode (or `./run-cssim.sh ui` for GUI/UI mode)

#### Fixed
- The custom ion loader would allow only integer atomic mass. This was wrong, now doubles are accepted too.
- Same fix was applied to custom materials

### [4.1.0] - 2025-05-21 (M. P.)

#### Added
* Now the user can specify custom materials. Ref. new `custom-materials.json` configuration file. Of course the material is FIRST built with G4NIST database, this custom framework is a fallback.

#### Fixed
* Macros for ion custom definition had a minor malfunction, this has been fixed.

### [3.8.7] - 2025-05-20 (M. P.)

#### Added
* Now the user can define custom particles as triplets, identified by a NAME, of : {`Z`, `A`, `excitationEnergy`}. Caveats: a) there is a new json configuration file for this: `custom-particles.json`. Names must be unique b) Names must NOT overlap the ones in the Geant4 particle database (or they'll be neglected, and the database's ones will prevail). c) Macro commands, demonstrated (currently commented out) in the `run.mac`, allow macro operation for this option in case you don't want to use the json file.

#### Changed
* In consequence of the new functionalities, the input beam breakdown CSV is now enriched, with new 'Alias' and 'isCustom' columns, eg:
```
Particle;Alias;isCustom;Energy;Flux;Count;Pct
C16;Carbon;Y;10.000000;1.000000;5974;5.00
proton;proton;N;10.000000;1.000000;5929;5.00
```

* INTERNAL: many parameters were moved from env variables to command line arguments

#### Fixed
* the geometry cleanup in case of bad loading was incomplete. this has been fixed
* The program would crash (core dump) in front of an unrecognized particle, now a cleaner error message is visible.
* The program would crash (core dump) in front of an unrecognized material, now a cleaner error message is visible.

### [3.8.5] - 2025-05-09 (M. P.)

#### Changed
* Now if the geometry load from json routine fails because of some inconsistencies, eventual partial progress in the building of the geometry is cleared, so eventually macros can build from scratch

### [3.8.4] - 2025-05-08 (M. P.)

#### Added
* Now a CSV is saved also for the survived primary particles, and for the beam composition breakdown.

#### Changed
* Run script streamlined and amended from many inaccuracies (redundancies, variables with misleading names etc)
* Run script editable section made more compact and clear

#### Fixed
* Internal C++ major variable streamlining and renamings. Many variables had misleading names, this has been improved.

### [3.8.3] - 2025-05-07 (M. P.)

#### Added
* Now the end of run summary is printed to a CSV file
* The command line parsing framework has been introduced again, streamlining the access to the command line (before, some restriction was in place for the UI mode, now this has been addressed).

### [3.8.2] - 2025-05-07 (M. P.)

#### Added
* Now each threads prints its activity summary, i.e. how many steps did it process. Useful to check on the unbalancement (normal, but it's better to keep an eye).

#### Changed
* CsvMerger embedded in the program is now DISABLED and probably it will be permanently removed soon. The related environment variables have been removed from the run script
* HIGHLY streamlined the CSV machinery, this is just preparatory to make upcoming changes EASY.

### [3.8.1] - 2025-05-06 (M. P.)

#### Fixed
* Removed some dead code

### [3.8.0] - 2025-05-05 (M. P.)

#### Added
* Now there is no "detector" per se. Simply, all the layers are homogeneously defined in terms of a richer set of attributes. As a consequence, these following enhancements came natural.
* Now, the tracking of the surviving primary particles can be done in ALL layers
* Now the concept of detector is no more hardcoded. Each layer is now qualified by two attributes that characterize its behavior. And, from this angle, the layers we'll use as detectors behave different from the others, but this difference can be stated at configuration file for each layer, i.e. we can have as many detectors as we want, placed where we want. Ref. geometry configuration file for the very specific flags and their values.

### [3.7.9] - 2025-05-05 (M. P.)

#### Added
* Now the beam configuration can be set also via macro

#### Changed
* Internal code additional modularization and reorganization

### [3.7.8] - 2025-05-04 (M. P.)

#### Added
* Added MASS field to the step data CSV
* Calculations for NormSv (radiations in normalized Sievert) are heavy, now there is an environment variable to eventually suppress them, `CSS_EVAL_RADIATIONS`.
* `CSS_WRRES_DEBUG' new environment variable to have super detailed debug of the WR calculation
* Out of the last layer, it is now tracked which PRIMARY events' particles made it, and with which energy (ranges - histogram).

### [3.7.7] - 2025-05-03 (M. P.)

#### Added
* Added Sievert/WT to the step data CSV AND to the EOR summary (WR is configurable in a new json file).
* Emphasized that layer names should NOT contain spaces
* Added a csv sorting utility (`csv_sorter.sh`)
* Added the following physics lists in the ones with smart verbosity suppression:
  `QGSP_BERT_HP`, `FTFP_BERT_HP`, `Shielding`, `QGSP_INCLXX_HP`, `QGSP_BIC_HP`
  (`QGSP_BIC_AllHP` was present already)
* Tracked in log which particle selection was actually used to launch (ie distinct eventId, trackId out of SelectParticle), group by particle type and energy
* Added process name in csv
* Now the particles are selected honoring the WEIGHTS implicitly defined by the FLUX

#### Changed
* The CSV MERGER embedded in the simulator, ref. open points, behaves improperly with large files, so it is now disabled by default in the run script. To cover the same need a scripted utility has been released, `csv_merger.sh` which accepts, as input, the timestamp of a run, and generates the merged CSV file. This is slim and performant, but of course no primary key check.

* Macros syntax for new css specific command will be `/css/..something`, not anymore `/sim/..`

* Starting 3.7.7, the flag for enabling the per-thread summary is OFF. Functionality is still there, but only the global summary is visible by default.

### [3.7.6] - 2025-04-29 (M. P.)

#### Added
* Now the geometry can be specified in a mac file (ONLY if the `geometry.json` is not utilized)
* Added parent track id on CSV

#### Changed
* A few internal code optimizations
* Improvements in the run script
* Compiled against Geant4 v 11.3.2, xerces pointers were needed in the build script, and the env script points at the stack nbr 2.

### [3.7.5] - 2025-04-28 (M. P.)

#### Added
* Now main setup referenced is 250428, where we added the flag needed to enable the OpenGL graphic interface, which we started to try. the main, now, if in "ui" mode (ref the "op_mode" variable in it), has to environment variables one for initializing the UI the one for running a load right after the GUI comes up. Attention: the UI takes ten mins to come up.

### [3.7.4] - 2025-04-28 (M. P.)

#### Added
* Now the world's origin is configurable in the geometry.json config file.
However, at least for the moment, no values are admitted, other than 0-0-0
Geant4 DOES NOT SUPPORT positions other than 0-0-0 for the world object. The application exposes this parameter as configurable just in case things change in future. And the workaround (creating an innerworld) would introduce complexity, and we won't do it, at least for the moment.

* To compensate the need for playing freely with the Z, we added a `CSS_CHART_Z_OFFSET` environment variable which, when specified, will add an offset at the root, i.e. upon CSV generation. HOWEVER BE INFORMED: this does not reflect inner calculation. It's only surfacing in the CSV. 

### [3.7.3] - 2025-04-28 (M. P.)

#### Added
* Added energyDeposited in the material, both to summary reports and in CSV

#### Changed
* Changed the logic determining the entrance in detectory, as it was inconsistent. Ref. notes in `SteppingAction.cc` for full detail.

* The ParticleDataManager now prints the EOR summary in a more efficient way.

* Now, by default, the per-thread summary is disabled (changed a setting in the run script).

* summary threads+global analyzer available in a version which checks ENERGY too. But it is in docs folder because it does not work (ref. details in Open Points section).

* Now the run script has thread summary disabled by default

* Now the cleanup scripts clears the logs, the temp CSVs, the out/r*

#### Fixed
* Summary threads+global analyzer covers now strings like Cs128[187.570X] and Dy159[177.614]


### [3.7.2] - 2025-04-26 (M. P.)

#### Added
* Unified summary report. And a script to analyze its quality (compared to per-thread summaries) has been provided (analyzeSummary.sh: you have to pass it the log file name - by default it will pick up the latest). Note: the map for treating the detector step processing has been kept in the specific thread. This is both CONSISTENT (because event+thread uniquely represents the particle, and one event is ENTIRELY processed in a single thread) and efficient (because each thread will deal with smaller maps for searches).

* Now, with these new environment variables in the run script, you can toggle on/off the end of run summaries:

```
export CSS_GLOBAL_SUMMARY=ON
export CSS_THREAD_SUMMARY=OFF
```

* After usage, i.e. after printing the summary, the data structures utilized for the end of run summary are CLEARED.
It might seem useless, but it could come handy in case we add post-processing activities (optimizes performance).

* This new environment variable in the run script allows to decide whether or not we want to count the entrance of the particles in the detector only once or in all occurrences.
```
# Set to ON if you want the particle entries in the detector to be accounted for
# at all occurrences (with OFF, only first entrance is counted).
# OFF is the default only because this was the suggestion in the code at first time.
export CSS_COUNT_ALL_IN_DETECTOR=OFF
```

#### Changed
* Now in the log also the G4 thread id is shown (-1 for the main, 0, 1, 2, for others)
* The PRIMARY KEY ("has entered") for treating the detector step processing is no more trackId alone, but the pair eventId, trackId. This seems appropriate as this pair is unique overall and also in the specific thread.

### [3.7.1] - 2025-04-25 (M. P.)

#### Changed
* Now, with the new environment variable `CSS_PHYSLIST_VERBOSITY` (ref. run script) the physics list initialization is silent. NOTE: this works ONLY if the physics is QGSP_BIC_AllHP or a few others, ref. open points.

### [3.7.0] - 2025-04-24 (M. P.)

#### Added
* Application logging streamlined, and an environment variable in the run script allows controlling the level: `CSS_LOG_LEVEL` (note: this is related only to THIS application's logging, NOT to the G4 inner activities).

* The final per-thread summary now prints all at once, more readable (as it does not interlace with siblings).

#### Changed
* Minor internal code optimizations.

* Now ALL the json configuration files show numbers with decimals (i.e. reals instead of integers) where appropriate.

### [3.6.0] - 2025-04-23 (M. P.)

#### Added
* New `CSS_CSV_ENHANCED` (=ON/OFF) environment variable in the run script, which will toggle these additional columns in the CSV (if CSV is enabled): xPos, yPos, mom. dir. x, mom. dir. y, mom. dir. z.

#### Changed
* Debug log for the flow is suppressed. It has no sense at all, with the CSV we now have in place.

* Now the program will raise an exception in case of non-uniqueness of the geometry layers NAMES.

* Added a comment in the geometry configuration file, reminding the user that the layer names MUST be different.

### [3.5.0] - 2025-04-23 (M. P.)

#### Changed
* Now the final summary is per-layer. Ref. this Chat GPT [conversation thread](https://chatgpt.com/share/68076da9-5594-8012-97d6-41eeed6349bb)

#### Fixed
* Internal code redundancies addressed.

### [3.4.0] - 2025-04-21 (M. P.)

#### Added
* Now in the merged CSV a header is inferred out of this structure visible in the thread's specific file
```
#column double TrackId
#column double StepNbr
#column double ThreadId
```

#### Changed
* Internal refactorings (eg the 'data' token, used for the csvs and referenced in the main, is no longer duplicated, it is only hardcoded once, in the main).

* The CSV generation now can be enabled/disabled with an environment variable in the run script

* The run script does not create the csv directory anymore. It is the program that does so.

#### Fixed
* The CSV sorting had a BUG: the sorting was string-based and not number-based. Now fixed.

### [3.3.0] - 2025-04-21 (M. P.)

#### Changed
* Now a CSV MERGER can be activated (active by default, ref. variables in the RUN script) which will generate a single CSV out of the per-thread ones.

* Now the CSV contains way more columns. Ref. notes above about the CSV, and about the meaning of the fields.


### [3.2.0] - 2025-04-20 (M. P.)

#### Added
* The simulator now creates a CSV of kinetic and residual energies over Z, although one file for each thread.

### [3.1.0] - 2025-04-19 (M. P.)

#### Added
* The `run.mac` now shows how to increase number of threads
* The detector thickness is now configurable (20.0 mm default if not specified)
* The detector copy number is now configurable (last layer's + 1 if not specified)
* The offset between world and layers is now configurable (10.0 mm default if not specified)
* BEAM configuration now has defaulting mechanisms in place for initial position and direction momentum, to avoid redundancies. Plus, the default momentum directions is the one which ALSO applies when the specified one is too small (before, it was hardcoded). The old format for `beam.json` is still supported but HIGHLY DEPRECATED.

#### Changed
* Minor internal code optimizations and hardcoding removals.
* The configuration files are more accurate as it relates to data types (eg 1.0 instead of 1 if it's not integer. PLEASE STICK TO THIS SYNTAX)
* Refactored the run script so that now the launch happens in NOHUP BACKGROUND
* Updated the CMakeLists.txt so that now the macros are no longer duplicated, they are now pointed in the proj directory.

#### Fixed
* Beam configuration JSON file was loaded once per thread, now it is loaded just ONCE.


### [3.0.0] - 2025-04-17 (M. P.)

#### Changed
* In `PrimaryGeneratorAction.cc`, the following section

```
fParticleGun->SetParticlePosition(G4ThreeVector(-0.5 * m, 0., 0.));
fParticleGun->SetParticleMomentumDirection(G4ThreeVector(1., 0., 0.));
```
was modified to:

```
fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., -1.0 * m));
fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
```

This made the entire thing work. FINALLY WE HAVE STUFF GOING ON. Also, now these parameters are externalized/configurable in the beam configuration file.

* The environment variable `CSS_FLOW_DEBUG', configured in the RUN script, when set to `ON`, enables deep flow debugging. SUPERVERBOSE, do it only with 1, 2, max 10 particles.

* Now the `layers.json` is renamed to `geometry.json` and it holds more parameters related to the geometry as a whole.

* Now the XY symmetry is not hardcoded anymore, it is just configurable in `geometry.json`

* Now also the max step in the world and in the layers is configurable (if not specified in the json, it defaults to what is driven by the selected physics).

### [2.1.0] - 2025-04-17 (M. P.)

#### Changed
* Now at runtime the config directory is configurable in an environment variable: `CSS_CONFIG_DIR`,  which defaults to `./config` if not specified.

## Change Log

### [2.0.0] - 2025-04-17 (M. P.)

#### Changed
* In the hope of obtaining relevant results (like CSVs, etc), Chat GPT suggested a few modifications to: SteppingAction.hh, SteppingAction.cc, RunAction.cc (ref. [conversation thread](https://chatgpt.com/share/67fd279a-3514-8012-9a4b-f2a580da86cd))

### [1.0.1] - 2025-04-16 (M. P.)

#### Added
* Now the application will log Geant4 version, tool version and author, physics list
* Now the physics list is configurable (new config file: appconfig.json)

#### Fixed
* At runtime, the geometry configuration file was not found. Fixed by adding this line to the `CMakeLists.txt`, which will make sure that a copy of `config` is created in the build directory:
```
  file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})
```
* Error `G4Exception , For multi-threaded version, define G4VUserPrimaryGeneratorAction in G4VUserActionInitialization` fixed by adding proper includes and after reorganizing the calls out of simple google searches.

* Error `terminate called after throwing an instance of 'G4HadronicException'` fixed by adding the following to the run script.

```
. ${G4_INSTALL_ROOT}/bin/geant4.sh
export G4PARTICLEHPDATA=$GEANT4_DATA_DIR/G4TENDL1.4
```

### [1.0.0] - 2025-04-15 (M. P.)

#### Changed
* Now configurations and macros are the ones shared by F. M. Soccorsi on Apr. 15, 2025. The ones initially created by Chat GPT are under docs folder.

#### Fixed
* Improper comments syntax in the CMake file, `//` not accepted. Fixed by replacing with `#`.

* Many, many build errors occurred. Fixed by adding the following property to the CMAKE command:

```bash
-DGeant4_DIR=${G4_INSTALL_DIR}/lib64/cmake/Geant4/Geant4Config.cmake
```

* Error `could not find git for clone of json-populate` fixed by replacing, in `CMakeLists.txt`:

```
target_link_libraries(CosmicShieldSimulation PRIVATE nlohmann_json::nlohmann_json)
```
with:
```
include_directories(${PROJECT_SOURCE_DIR}/include)
```

This told the compiler to search for headers only in `./include` and not on the web. Therefore, the build script now downloads (`wget`) the `https://github.com/nlohmann/json/releases/latest/download/json.hpp` file and copies it locally under `./include/nlohmann` prior to moving on.

* Many build errors were fixed by adding, in `DetectorConstruction.hh`, the `#include "G4SystemOfUnits.hh"` statement after the `"globals.hh"` line.

* A build error was fixed by commenting out, in `CosmicShieldSimulation.cc`, the code line
`"G4SteppingVerbose::UseBestUnit verbose;" (it is optional)`.

* The `PrimaryGeneratorAction.cc:29:33: error: ambiguous overload for ‘operator=’` was fixed by modifying `PrimaryGeneratorAction.cc`. Specifically: add `#include "Randomize.hh"` on top and also changing:

```
p.name = item["particle"];
```

to

```
p.name = G4String(item["particle"].get<std::string>());
```

* Many errors like: `error: ‘static_cast’ from type ‘const G4UserSteppingAction*’ to type ‘SteppingAction*’ casts away qualifiers` fixed by modifying `RunAction.cc` and `SteppingAction.cc` with C-specific syntax needs, out of simple Google searches.

### [1.0.0alpha] - 2025-04-15 (Chat GPT)
First release, exactly how it came out of [Chat GPT's draft on Apr. 15, 2025](https://chatgpt.com/share/67fd279a-3514-8012-9a4b-f2a580da86cd)

