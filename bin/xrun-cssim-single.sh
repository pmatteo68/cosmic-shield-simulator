#!/bin/bash

. ../../../../scripts/bin/g4env-s3.sh
. ./bin/env-cssim.sh
. ./bin/g4-csv-env.sh

count_params=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    -om|--opmode) op_mode="$2"; let count_params=${count_params}+1; shift 2 ;;
    -ac|--appconf) app_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -cm|--custmat) cust_mat_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -ge|--geom) geom_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -cp|--custpart) cust_part_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -be|--beam) beam_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -wr|--wrres) wrres_cfg="$2"; let count_params=${count_params}+1; shift 2 ;;
    -bm|--macro) mac_file="$2"; let count_params=${count_params}+1; shift 2 ;;
    -um|--uimacro) ui_mac_file="$2"; let count_params=${count_params}+1; shift 2 ;;
    -so|--out) out_dir="$2"; let count_params=${count_params}+1; shift 2 ;;
    -we|--weightexcl) weight_exclusions="$2"; let count_params=${count_params}+1; shift 2 ;;
    -sd|--stepdata) save_stepdata="$2"; let count_params=${count_params}+1; shift 2 ;;
    -ci|--stepdcsvimpl) stepd_csvimpl="$2"; let count_params=${count_params}+1; shift 2 ;;
    -er|--evalradiat) eval_radiations="$2"; let count_params=${count_params}+1; shift 2 ;;
    -rt|--runtstamp) run_tstamp="$2"; let count_params=${count_params}+1; shift 2 ;;
    -sl|--log) log_file="$2"; let count_params=${count_params}+1; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done

batch_mode=batch
batch_exp_params=15

ui_mode=ui
#ui mode params are the same as batch, plus one: the uimacro
let ui_exp_params=${batch_exp_params}+1

if [ "x${op_mode}" == "x" ]; then
  echo "Argument missing: opmode"
  exit 1
else
  if [ "x${op_mode}" != "x${ui_mode}" ] && [ "x${op_mode}" != "x${batch_mode}" ]; then
    echo "Unknown opmode (found: $op_mode, must be '${ui_mode}' or '${batch_mode}'.)" >&2
    exit 1
  fi
  if [ "x${op_mode}" == "xbatch" ]; then
    if [ $count_params -lt $batch_exp_params ]; then
      echo "Insufficient arguments (found: $count_params, expected: $batch_exp_params)."
      exit 1
    fi
  fi
  if [ "x${op_mode}" == "xui" ]; then
    if [ $count_params -lt $ui_exp_params ]; then
      echo "Insufficient arguments (found: $count_params, expected: $ui_exp_params)."
      exit 1
    fi
  fi
fi

PROJ_DIR=../proj
##run_tstamp=$(date +"%Y%m%d%H%M%S%3N")
#run_tstamp=$(date +"%y%m%d%H%M%S")
#log_file_name=logs/css-v${CSS_VER}-${run_tstamp}.log
#log_file=${PROJ_DIR}/${log_file_name}

echo
echo "====================================================="
echo "Cosmic Shield Simulator v. ${CSS_VER} execution shell"
echo "====================================================="

echo "Setting Geant4 environment ..."
echo "(ref. ${G4_INSTALL_ROOT}/bin/geant4.sh)"
. ${G4_INSTALL_ROOT}/bin/geant4.sh

#echo "debug datasets:"
#ls -lart $GEANT4_DATA_DIR/
#echo

if [ -d "${BUILD_DIR}" ]; then
  echo "Build directory (${BUILD_DIR}) found."
  cd ${BUILD_DIR}
else
  echo -e "Build directory (${BUILD_DIR}) \033[1;5;31mNOT FOUND\033[0m."
  pwd
  echo "Can't compute. Please build, first."
  exit 1
fi

#echo GEANT4_DATA_DIR=$GEANT4_DATA_DIR
# export G4NEUTRONHPDATA=$GEANT4_DATA_DIR/G4NDL4.7.1
# export G4LEDATA=$GEANT4_DATA_DIR/G4EMLOW8.7
# export G4LEVELGAMMADATA=$GEANT4_DATA_DIR/PhotonEvaporation6.1
# export G4RADIOACTIVEDATA=$GEANT4_DATA_DIR/RadioactiveDecay6.1.2
# export G4PARTICLEXSDATA=$GEANT4_DATA_DIR/G4PARTICLEXS4.1
# export G4PIIDATA=$GEANT4_DATA_DIR/G4PII1.3
# export G4REALSURFACEDATA=$GEANT4_DATA_DIR/RealSurface2.2
# export G4SAIDXSDATA=$GEANT4_DATA_DIR/G4SAIDDATA2.0
# export G4ABLADATA=$GEANT4_DATA_DIR/G4ABLA3.3
# export G4INCLDATA=$GEANT4_DATA_DIR/G4INCL1.2
# export G4ENSDFSTATEDATA=$GEANT4_DATA_DIR/G4ENSDFSTATE3.0
# export G4CHANNELINGDATA=$GEANT4_DATA_DIR/G4CHANNELING1.0
export G4PARTICLEHPDATA=$GEANT4_DATA_DIR/G4TENDL1.4
#echo G4PARTICLEHPDATA=$G4PARTICLEHPDATA
#echo G4NEUTRONHPDATA=$G4NEUTRONHPDATA
#echo G4LEDATA=$G4LEDATA
#echo G4ENSDFSTATEDATA=$G4ENSDFSTATEDATA
#echo G4PIIDATA=$G4PIIDATA
#echo "end debug vars"

#CSS_CONFIG_DIR=${PROJ_DIR}/config
#APP_CONFIG=${CSS_CONFIG_DIR}/appconfig.json
#MATERIALS_CONFIG=${CSS_CONFIG_DIR}/custom-materials.json
#GEOM_CONFIG=${CSS_CONFIG_DIR}/geometry.json
#PARTICLES_CONFIG=${CSS_CONFIG_DIR}/custom-particles.json
#BEAM_CONFIG=${CSS_CONFIG_DIR}/beam.json
#WR_RES_CONFIG=${CSS_CONFIG_DIR}/wrresolver.json
APP_CONFIG=${app_cfg}
MATERIALS_CONFIG=${cust_mat_cfg}
GEOM_CONFIG=${geom_cfg}
PARTICLES_CONFIG=${cust_part_cfg}
BEAM_CONFIG=${beam_cfg}
WR_RES_CONFIG=${wrres_cfg}

#macros_dir=${PROJ_DIR}/macros
RUN_MACRO=${mac_file}
#RUN_MACRO=${macros_dir}/run.mac
#REL_CSS_OUT_DIR=out/r${run_tstamp}
#CSS_OUT_DIR=${PROJ_DIR}/${REL_CSS_OUT_DIR}
CSS_OUT_DIR=${out_dir}

echo "======= Cosmic Shield Simulator v. ${CSS_VER} Run Technical Sheet ============" > ${log_file}
echo >> ${log_file}
echo "Timestamp: ${run_tstamp}" >> ${log_file}
echo >> ${log_file}
echo "Geant4 install root: ${G4_INSTALL_ROOT}" >> ${log_file}
echo >> ${log_file}
echo "Geant4 environment: ref. ${G4_INSTALL_ROOT}/bin/geant4.sh" >> ${log_file}
echo >> ${log_file}
echo "G4PARTICLEHPDATA=${G4PARTICLEHPDATA}" >> ${log_file}
echo >> ${log_file}
echo "------ Application configuration:" >>${log_file}
cat ${APP_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ Geometry configuration:" >>${log_file}
cat ${GEOM_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ Custom materials configuration:" >>${log_file}
cat ${MATERIALS_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ Custom particles configuration:" >>${log_file}
cat ${PARTICLES_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ Beam configuration:" >>${log_file}
cat ${BEAM_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ WR Resolution configuration:" >>${log_file}
cat ${WR_RES_CONFIG} >>${log_file} 2>&1
echo >> ${log_file}
echo "------ Run macro:" >>${log_file}
cat ${RUN_MACRO} >>${log_file} 2>&1
echo >> ${log_file}
echo >> ${log_file}
#echo "Creating the reports directory (${CSS_OUT_DIR})..." >> ${log_file}
#mkdir -p ${CSS_OUT_DIR} >>${log_file} 2>&1
echo
echo "========== LOG - BEGIN =========" >> ${log_file}

# disabled # Embedded CSV Merger - disabled as of 3.8.2
# disabled # The csv merger embedded in the simulator worked fine only with small files.
# disabled # So it is disabled. Soon will be completely taken out.
# disabled export CSS_CSV_MERGING=OFF
# disabled export CSS_CSV_INFER_HEADER=ON
# disabled export CSS_CSVMERGER_IN_COLSEP=","
# disabled export CSS_CSVMERGER_OUT_COLSEP=";"
# disabled export CSV_MERGER_THREADS=4
# disabled export CSS_CSV_KEYFIELDS=0,1,2
# disabled export CSS_CSV_SORTBY=${CSS_CSV_KEYFIELDS}
# Embedded CSV Merger - end

########### Fine grain parameters, edit only if you are well aware as to what they do

# KPI evaluation parameters (how many layers to discard, starting from the last one; energy OUT or energy IN)
export CSS_KPI_LAYERS_TO_DISCARD=1
export CSS_KPI_EVAL_DIRECTION=OUT

# Global KPIs summary
export CSS_GLOB_KPIS_CSV_SEP=";"
export CSS_GLOB_KPIS_CSV_FNAME=glob_kpis_${run_tstamp}.csv

# Global geometry summary
export CSS_GLOB_GEOM_SUMM_CSV_SEP=";"
export CSS_GLOB_GEOM_SUMM_CSV_FNAME=summ_geom_glob_${run_tstamp}.csv

# Global particles summary
export CSS_GLOB_PART_SUMM_CSV_SEP=";"
export CSS_GLOB_PART_SUMM_CSV_FNAME=summ_part_glob_${run_tstamp}.csv

# survivor primary particles
# NOTE: this can be enabled ONLY if CSS_GLOBAL_SUMMARY is 'ON'
export CSS_SURVP_SUMM_CSV_SEP=";"
export CSS_SURVP_SUMM_CSV_FNAME=summ_survp_${run_tstamp}.csv
export CSS_SURVP_SUMM_RNGDIGITS=3

# If ON, a report is created about the beam's detailed particle composition
export CSS_BEAMBKD_CSV_SEP=";"
export CSS_BEAMBKD_CSV_FNAME=beam_bkd_${run_tstamp}.csv
export CSS_BEAMBKD_PCTDIGITS=2

#step data: add all fields
export CSS_STEPDATA_CSV_ENHANCED=ON
#step data custom impl: separator
#export CSS_STEPDATA_CSV_SEP=";"

################################## BEGIN EDITABLE SECTION #########################

# Lists the layers you want to exclude from shield weight evaluation
export CSS_WEIGHT_EXCLUSIONS=${weight_exclusions}

# Enable the creation of the step data detailed CSV CSS_SAVE_STEPDATA
# CAUTION, very verbose, occupies large space
CSS_SAVE_STEPDATA=${save_stepdata}

# With 'G4', one csv per thread is created, with 'custom' a merged one
# (but simulation may take longer)
#CSS_STEPDATA_CSV_IMPL=custom
#CSS_STEPDATA_CSV_IMPL=G4
CSS_STEPDATA_CSV_IMPL=${stepd_csvimpl}

#Per-thread particles summary
CSS_THREAD_PART_SUMMARY=OFF

# Global KPIs
CSS_GLOBAL_KPIS=ON

# Global particles summary
CSS_GLOBAL_PART_SUMMARY=ON

# Global geometry summary
CSS_GLOBAL_GEOM_SUMMARY=ON

# survivor primary particles
# NOTE: this can be enabled ONLY if CSS_GLOBAL_PART_SUMMARY is 'ON'
CSS_SURVIVORPART_SUMMARY=ON
#Survived particles breakdown bin size (number of MeV brackets)
export CSS_SURV_PTC_BINSIZE=10

# If ON, a report is created about the beam's detailed particle composition
CSS_BEAM_BREAKDOWN=ON

# The following ones apply ONLY if if saving step data is ON (ref. property above)
# (z offset is in mm)
export CSS_CHART_Z_OFFSET=0.0

# enable/disable calculation of radiations
# (and they will be visible in whichever enabled report containing this field)
CSS_EVAL_RADIATIONS=${eval_radiations}

####################### LOGGING
#Levels are the G4Logging values
#EMERGENCY: 1
#ALERT: 2
#CRITICAL: 3
#ERROR: 4
#WARNING: 5
#INFO: 6
#DEBUG: 7
#TRACE: 8
CSS_LOG_LEVEL=6
# The following works only if physics list is
# among the ones visible in the src/PhysicsManager.cc file.
# Any other list will be correctly loaded, but its verbosity will be
# not affected by this setting
export CSS_PHYSLIST_VERBOSITY=0
# If the css log level is at least 7, and this is ON, you'll see super-detailed
# debug of the WR calculations
export CSS_WRRES_DEBUG=OFF

################################ Editable Section - END ######################

displayOutArtifactsMData() {
  if [ "x${CSS_BEAM_BREAKDOWN}" == "xON" ]; then
      echo "Beam particles breakdown: ${CSS_OUT_DIR}/${CSS_BEAMBKD_CSV_FNAME}"
  else
      echo "Beam particles breakdown: NO"
  fi
  echo "Per-thread particles summary: ${CSS_THREAD_PART_SUMMARY}"
  if [ "x${CSS_GLOBAL_GEOM_SUMMARY}" == "xON" ]; then
      echo "Global geometry summary: ${CSS_OUT_DIR}/${CSS_GLOB_GEOM_SUMM_CSV_FNAME}"
  else
      echo "Global geometry summary: NO"
  fi
  if [ "x${CSS_GLOBAL_PART_SUMMARY}" == "xON" ]; then
      echo "Global particles summary: ${CSS_OUT_DIR}/${CSS_GLOB_PART_SUMM_CSV_FNAME}"
      if [ "x${CSS_SURVIVORPART_SUMMARY}" == "xON" ]; then
          echo "Survived primary particles summary: ${CSS_OUT_DIR}/${CSS_SURVP_SUMM_CSV_FNAME}"
      else
          echo "Survived primary particles summary: NO"
      fi
  else
       echo "Global particles summary: NO"
  fi
  if [ "x${CSS_GLOBAL_KPIS}" == "xON" ]; then
       echo "Global KPIs: ${CSS_OUT_DIR}/${CSS_GLOB_KPIS_CSV_FNAME}"
  else
       echo "Global KPIs: NO"
  fi
  if [ "x${CSS_SAVE_STEPDATA}" == "xON" ]; then
    echo "Step data files: ${CSS_OUT_DIR} (implementation: ${CSS_STEPDATA_CSV_IMPL})"
  else
    echo "Step data files: NO"
  fi
  echo "Evaluate radiations: ${CSS_EVAL_RADIATIONS}"
  echo "Shield weight exclusions: ${CSS_WEIGHT_EXCLUSIONS}"
}

#mode can be batch or ui
#op_mode="ui"
#op_mode="batch"
echo "Launching simulation (mode: ${op_mode}, log level: ${CSS_LOG_LEVEL})" >>${log_file}
echo "Launching simulation (mode: ${op_mode}, log level: ${CSS_LOG_LEVEL})"
echo "Log: ${log_file}"
CSS_EXE=CosmicShieldSimulation
echo
if [ "x${op_mode}" == "xbatch" ]; then
  ######### Batch mode
  ## For memory analysis you may use: valgrind --leak-check=summary --track-origins=no ./${CSS_EXE} \ (instead of just ./${CSS_EXE})
  ## Also: note that heavier Valgrind options like --leak-check=full and --track-origins=yes may hit the performance severily. Use carefully.
  #nohup ./${CSS_EXE} \
  ./${CSS_EXE} \
    --css-log-level ${CSS_LOG_LEVEL} \
    --css-conf-app ${APP_CONFIG} \
    --css-conf-geom ${GEOM_CONFIG} \
    --css-conf-materials ${MATERIALS_CONFIG} \
    --css-conf-particles ${PARTICLES_CONFIG} \
    --css-conf-beam ${BEAM_CONFIG} \
    --css-conf-wrres ${WR_RES_CONFIG} \
    --css-output-dir ${CSS_OUT_DIR} \
    --css-global-kpis ${CSS_GLOBAL_KPIS} \
    --css-global-geom-report ${CSS_GLOBAL_GEOM_SUMMARY} \
    --css-global-part-report ${CSS_GLOBAL_PART_SUMMARY} \
    --css-thread-part-report ${CSS_THREAD_PART_SUMMARY} \
    --css-survivors-summary ${CSS_SURVIVORPART_SUMMARY} \
    --css-beam-breakdown ${CSS_BEAM_BREAKDOWN} \
    --css-eval-radiations ${CSS_EVAL_RADIATIONS} \
    --css-save-stepdata ${CSS_SAVE_STEPDATA} \
    --css-stepdata-csv-impl ${CSS_STEPDATA_CSV_IMPL} \
    --css-mac ${RUN_MACRO} >> ${log_file} 2>&1
    #--css-mac ${RUN_MACRO} >> ${log_file} 2>&1 &
  displayOutArtifactsMData
  #ppid=$!
  #echo "Process launched in background (pid: ${ppid})"
  #displayOutArtifactsMData
  #echo
  #echo "Check log with:"
  #echo "tail -f ${log_file}"
  #echo
else
  ######### UI mode
  #CSS_UI_INIT_MACRO=${maXcros_dir}/init_vis.mac
  CSS_UI_INIT_MACRO=${ui_mac_file}
  displayOutArtifactsMData
  echo
  ./${CSS_EXE} \
    -css-ui \
    --css-log-level ${CSS_LOG_LEVEL} \
    --css-vismac ${CSS_UI_INIT_MACRO} \
    --css-conf-app ${APP_CONFIG} \
    --css-conf-geom ${GEOM_CONFIG} \
    --css-conf-materials ${MATERIALS_CONFIG} \
    --css-conf-particles ${PARTICLES_CONFIG} \
    --css-conf-beam ${BEAM_CONFIG} \
    --css-conf-wrres ${WR_RES_CONFIG} \
    --css-output-dir ${CSS_OUT_DIR} \
    --css-global-kpis ${CSS_GLOBAL_KPIS} \
    --css-global-geom-report ${CSS_GLOBAL_GEOM_SUMMARY} \
    --css-global-part-report ${CSS_GLOBAL_PART_SUMMARY} \
    --css-thread-part-report ${CSS_THREAD_PART_SUMMARY} \
    --css-survivors-summary ${CSS_SURVIVORPART_SUMMARY} \
    --css-beam-breakdown ${CSS_BEAM_BREAKDOWN} \
    --css-eval-radiations ${CSS_EVAL_RADIATIONS} \
    --css-save-stepdata ${CSS_SAVE_STEPDATA} \
    --css-stepdata-csv-impl ${CSS_STEPDATA_CSV_IMPL} \
    --css-mac ${RUN_MACRO}
fi
