#!/bin/bash

#################### EDITABLE SECTION - BEGIN ###########

#Careful with ON - uses lots of disk space
sav_stepdata=ON

#If stepdata are saved, this is the implementation.
#Can be G4 or custom. With G4, there is one csv per thread.
#With custom, one single file (simulation performance may be affected, but at the end no need to merge)
stepd_csv_impl=G4
#stepd_csv_impl=custom

#If stepdata are saved, here you can choose whether or not you want to merge them
#NOTE: if step csv impl is custom, any merging is just skipped regardless of this variable
mrg_stepdata=ON

#If stepdata are saved, and if merging is on, if this flag is ON then the raw per-thread files are deleted after merging.
del_rawsd_if_mrg=ON

# Layers you do NOT want to count, in the weight evaluation
wgt_exclusions=Detector,detector

#Careful: uses lots of CPU
eval_radiations=ON

#################### EDITABLE SECTION - END #############

echo "This is HIGHLY impactful on the environment."
read -p "Are you sure you want to continue? (y/n): " choice
case "${choice}" in
  y|Y ) echo "Proceeding...";;
  n|N ) echo "Aborting."; exit 0;;
  * ) echo "Invalid input. Aborting."; exit 1;;
esac

#Pointless to merge if the stepdata csv implementation is custom
#as in that case the file is already just one
if [ ! "x${stepd_csv_impl}" == "xG4" ]; then
  mrg_stepdata=OFF
fi

script_dir="$(dirname "$(readlink -f "$0")")"
cd "${script_dir}"
. ./bin/env-cssim-tr.sh
tr_data_root=${CSS_TR_DATA_HOME}

inputs_found=0
input_is_archive=false
if [ "x$1" == "x" ]; then
  echo "Error: Please specify the test configurations directory or archive." >&2
  exit 1
else
  dir_or_arc=$1
  if [ -d "${dir_or_arc}" ]; then
    let inputs_found=${inputs_found}+1
    runner_root=${dir_or_arc}
  else
    arc_path=${dir_or_arc}
    if [ -f "${arc_path}" ]; then
      echo "./bin/deflate-testrunner-archive.sh ${tr_data_root} ${arc_path}"
      runner_root=$(./bin/deflate-testrunner-archive.sh ${tr_data_root} "${arc_path}" 2>/dev/null)
      if [[ -n "${runner_root}" && -d "${runner_root}" ]]; then
        input_is_archive=true
        let inputs_found=${inputs_found}+1
      fi
    fi
  fi
fi
if [ $inputs_found -lt 1 ]; then
  echo "Error: the input resources specified are either not existing, or not accessible, or not of the expected type." >&2
  exit 1
fi


if [ "x$2" == "x" ]; then
  echo "Error: Missing magic parameter." >&2
  exit 1
fi

echo "===================================================="
echo "Cosmic Shield Simulator - Testrunner execution shell"
echo "===================================================="

tr_tstamp=$(date +"%y%m%d%H%M%S")
#tr_log=${script_dir}/logs/css-testrunner-${tr_tstamp}.log
log_dir=${script_dir}/logs
if [ -d "${runner_root}/logs" ]; then
  log_dir=${runner_root}/logs
fi
tr_log=${log_dir}/css-testrunner-${tr_tstamp}.log
echo
echo "Test configurations: ${runner_root}"
if [ "x${input_is_archive}" == "xtrue" ]; then
  echo "(deflated from: ${arc_path})"
fi
if [ "x${sav_stepdata}" == "xON" ]; then
  echo "Save step data: ${sav_stepdata} (merge: ${mrg_stepdata})"
  if [ "x${mrg_stepdata}" == "xON" ]; then
    echo "Delete raw step data after merge: ${del_rawsd_if_mrg} (impl: ${stepd_csv_impl})"
  fi
else
  echo "Save step data: ${sav_stepdata}"
fi
echo "Evaluate radiations: ${eval_radiations}"

nohup ./bin/xrun-cssim-multi.sh \
  -rd "${runner_root}" \
  -sd ${sav_stepdata} \
  -ci ${stepd_csv_impl} \
  -er ${eval_radiations} \
  -ms ${mrg_stepdata} \
  -we ${wgt_exclusions} \
  -dr ${del_rawsd_if_mrg} >> ${tr_log} 2>&1 &
ppid=$!
echo "Testrunner loop launched in background (pid: ${ppid})"
echo
echo "Check log with:"
echo "tail -f ${tr_log} | grep \"OPMARKER\""
echo "(you can omit '| grep ...' to see some detail more)"
echo

cd -
