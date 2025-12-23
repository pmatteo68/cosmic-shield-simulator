#!/bin/bash

. ./bin/g4-csv-env.sh
#echo "This is HIGHLY impactful on the environment."
#read -p "Are you sure you want to continue? (y/n): " choice
#case "${choice}" in
#  y|Y ) echo "Proceeding...";;
#  n|N ) echo "Aborting."; exit 0;;
#  * ) echo "Invalid input. Aborting."; exit 1;;
#esac
count_params=0
exp_params=7
while [[ $# -gt 0 ]]; do
  case "$1" in
    -rd|--runner-dir) runner_root="$2"; let count_params=${count_params}+1; shift 2 ;;
    -sd|--stepdata) save_stepdata="$2"; let count_params=${count_params}+1; shift 2 ;;
    -ci|--stepdcsvimpl) stepd_csvimpl="$2"; let count_params=${count_params}+1; shift 2 ;;
    -dr|--delrawstepd) del_raw_stepdata="$2"; let count_params=${count_params}+1; shift 2 ;;
    -er|--evalradiat) eval_radiat="$2"; let count_params=${count_params}+1; shift 2 ;;
    -we|--weightexcl) weight_exclusions="$2"; let count_params=${count_params}+1; shift 2 ;;
    -ms|--merge-stepdata) merge_stepdata="$2"; let count_params=${count_params}+1; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done
if [ $count_params -lt $exp_params ]; then
  echo "Usage:"
  echo "  $0 -rd <testrunner directory> -sd <save step data (ON|OFF)> -ms <merge step data (ON|OFF)> -dr <delete raw step data after merge (ON|OFF)> -er <evaluate radiations (ON|OFF)>"
  exit 1
fi

if [ ! -d "${runner_root}" ]; then
  echo "Directory '${runner_root}' does not exist." >&2
  exit 1
fi

evalConfigFile() {
  fname=$1
  dir_comm=$2
  dir_test=$3
  comm_file=${dir_comm}/${fname}
  test_file=${dir_test}/${fname}
  if [ -f "${test_file}" ]; then
    echo "${test_file}"
  else
    echo "${comm_file}"
  fi
}

echo
echo "===================================="
echo "Cosmic Shield Simulator - Testrunner"
echo "v. 1.2.0 - M. Picciau"
echo "===================================="
echo

#script_dir="$(dirname "$(readlink -f "$0")")"
#cd "${script_dir}"
curr_dir=$(pwd)

#num_tXests=$(find "${ruXnner_root}" -mindepth 1 -maxdepth 1 -type d | grep "test\-" | wc -l)
num_tests=$(./bin/list-testrunner-tests.sh "${runner_root}" 2>/dev/null | wc -l)

if [ ${num_tests} -gt 0 ]; then
  echo "Configurations: ${runner_root}"
  echo "Nbr of tests: ${num_tests}"
else
  echo "No test directory was found. EXIT"
  exit 0
fi

raw_stepdata_filepatt=$(./bin/printRawStepDataFPatt.sh)
runner_common_config=${runner_root}/common

idx=0
while [ ${idx} -lt ${num_tests} ]; do
  let idx=${idx}+1
  cur_test_root=$(./bin/list-testrunner-tests.sh "${runner_root}" 2>/dev/null | head -${idx} | tail -1)
  #cur_test_root=${runner_root}/test-${idx}
  echo "Performing test #${idx}: ${cur_test_root}"
  cur_test_in=${cur_test_root}/in
  is_input_valid=$(./bin/is-testinput-valid.sh ${runner_common_config} ${cur_test_in})
  if [ "x${is_input_valid}" == "xtrue" ]; then
    app_cfg_file=$(evalConfigFile appconfig.json ${runner_common_config} ${cur_test_in})
    cust_mat_file=$(evalConfigFile custom-materials.json ${runner_common_config} ${cur_test_in})
    geom_file=$(evalConfigFile geometry.json ${runner_common_config} ${cur_test_in})
    beam_file=$(evalConfigFile beam.json ${runner_common_config} ${cur_test_in})
    cust_part_file=$(evalConfigFile custom-particles.json ${runner_common_config} ${cur_test_in})
    wr_res_file=$(evalConfigFile wrresolver.json ${runner_common_config} ${cur_test_in})
    mac_file=$(evalConfigFile run.mac ${runner_common_config} ${cur_test_in})
    run_tmstamp=$(date +"%y%m%d%H%M%S")
    cur_test_out=${cur_test_root}/out
    mkdir "${cur_test_out}"
    cur_test_log=${cur_test_out}/sim-${idx}-${run_tmstamp}.log
    echo "[$(date)][OPMARKER][${idx}/${num_tests}][${cur_test_root}] TEST BEGIN"
    ${curr_dir}/bin/xrun-cssim-single.sh \
      -om batch \
      -ac ${app_cfg_file} \
      -cm ${cust_mat_file} \
      -ge ${geom_file} \
      -cp ${cust_part_file} \
      -be ${beam_file} \
      -wr ${wr_res_file} \
      -bm ${mac_file} \
      -so ${cur_test_out} \
      -sd ${save_stepdata} \
      -ci ${stepd_csvimpl} \
      -er ${eval_radiat} \
      -we ${weight_exclusions} \
      -rt ${run_tmstamp} \
      -sl ${cur_test_log}
    echo "[$(date)][OPMARKER][${idx}/${num_tests}][${cur_test_root}] TEST END"
    if [[ "x${save_stepdata}" == "xON" && "x${merge_stepdata}" == "xON" ]]; then
      echo "[$(date)][OPMARKER][${idx}/${num_tests}][${cur_test_root}] MERGE BEGIN"
      #echo "[$(date)][OPMARKER] Pre-merge I am in: $(pwd)"
      #echo "[$(date)][OPMARKER] Pre-merge curr_dir is: ${curr_dir}"
      #echo "[$(date)][OPMARKER] Pre-merge cur_test_out is: ${cur_test_out}"
      ${curr_dir}/csv_merger.sh \
        -i "${cur_test_out}" \
        -o "${cur_test_out}"
      if [ "x${del_raw_stepdata}" == "xON" ]; then
        echo "Deleting raw step data after merging it (filename pattern: ${raw_stepdata_filepatt})"
        #rm "${cur_test_out}"/data*step*.csv
        rm "${cur_test_out}"/${raw_stepdata_filepatt} > /dev/null 2>&1
      fi
      echo "[$(date)][OPMARKER][${idx}/${num_tests}][${cur_test_root}] MERGE END"
    fi
    echo "Done."
  else
    echo " Input data: NOT FOUND for this test - skip"
  fi
done

echo
echo "Testrunner loop COMPLETED."
echo "The requested tests (${num_tests}) were completed, results available under: ${runner_root}."

#cd - >/dev/null 2>&1

