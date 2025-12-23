#!/bin/bash

if [[ $# -ne 2 ]]; then
  echo >&2
  echo "Usage: $0 <common path> <test input path>" >&2
  echo >&2
  exit 1
fi

common_dir=$1
test_in_dir=$2

enumExpResources()
{
  echo appconfig.json
  echo custom-particles.json
  echo beam.json
  echo custom-materials.json
  echo geometry.json
  echo wrresolver.json
  echo run.mac
}

num_exp=$(enumExpResources | wc -l)
num_found=0
out_result=false

for cur_res in $(enumExpResources); do
  if [[ -f "${common_dir}/${cur_res}" || -f "${test_in_dir}/${cur_res}" ]]; then
    let num_found=${num_found}+1
  fi
done
if [ $num_found -eq $num_exp ]; then
  out_result=true
fi

echo $out_result
