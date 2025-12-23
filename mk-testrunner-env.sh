#!/bin/bash

echo "This is HIGHLY impactful on the environment."
read -p "Are you sure you want to continue? (y/n): " choice
case "${choice}" in
  y|Y ) echo "Proceeding...";;
  n|N ) echo "Aborting."; exit 0;;
  * ) echo "Invalid input. Aborting."; exit 1;;
esac

if ! [[ "$1" =~ ^[1-9][0-9]*$ ]]; then
  echo "Error: Please provide at least one argument which is a positive integer." >&2
  exit 1
fi


echo
echo "==================================================================="
echo "Cosmic Shield Simulator - Testrunner configuration creation utility"
echo "v. 1.0.0 - M. Picciau"
echo "==================================================================="
echo

num_tests=$1
script_dir="$(dirname "$(readlink -f "$0")")"
cd "${script_dir}"
. ./bin/env-cssim-tr.sh

curr_dir=$(pwd)

runner_root=${CSS_TR_DATA_HOME}
# ${curr_dir}/../../testrunner
echo "Target: ${num_tests}"
echo "Destination: ${runner_root}"
echo

runner_common_config=${runner_root}/common

echo "Deleting/recreating root testrunner configurations directory ..."
rm -rf ${runner_root} 2>/dev/null
mkdir ${runner_root} 2>/dev/null
echo "Done."

echo "Creating common testrunner configurations ..."
mkdir ${runner_common_config} 2>/dev/null
cp ${curr_dir}/config/*.json ${runner_common_config}/ 2>/dev/null
cp ${curr_dir}/macros/run.mac ${runner_common_config}/ 2>/dev/null
echo "Done."
echo "Creating testrunner log dir ..."
mkdir ${runner_root}/logs
echo "Done."

idx=0
while [ ${idx} -lt ${num_tests} ]; do
  let idx=${idx}+1
  echo "Creating test #${idx} configurations ..."
  cur_test_root=${runner_root}/test-${idx}
  mkdir ${cur_test_root}
  cur_test_in=${cur_test_root}/in
  #cur_test_out=${cur_test_root}/out
  #mkdir ${cur_test_in}
  #mkdir ${cur_test_out}
  cp -r ${runner_common_config}/ ${cur_test_in}/
  echo "Done."
done

echo "The process has completed successfully."
echo "The testrunner configurations requested (${num_tests}) were created under: ${runner_root}."

cd - >/dev/null 2>&1

