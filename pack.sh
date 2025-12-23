#!/bin/bash

. ./bin/env-cssim.sh

target_dir=~
echo "Packing project ..."
#proj_rel_dir=$(pwd | sed 's/\/proj//g')
#projects/cosmic-shield-sim/v4.1.1-250521/proj
#echo ${proj_rel_dir}
cur_proj_dir=$(basename "$(pwd | sed 's/\/proj//g')")
echo "Project dir is: ${cur_proj_dir}"
cd ../../../../
#echo "Moved to: $(pwd)"
arc_filename=css-${cur_proj_dir}.tar.gz
arc_fullpath=${target_dir}/${arc_filename}
if [ -f "${arc_fullpath}" ]; then
  echo "ERROR: file ${arc_fullpath} exists already: cannot overwrite"
  exit 1
else
  tar czf ${arc_fullpath} scripts/bin/g4env*.sh projects/cosmic-shield-sim/${cur_proj_dir}
  if [ -f "${arc_fullpath}" ]; then
    echo "The ${arc_fullpath} file has been created successfully"
    ls -lart ${arc_fullpath}
  else
    echo "ERROR: ${arc_fullpath} creation command succeeded, but the file seems not being there"
    exit 1
  fi
fi

cd ..
g4w_target=${target_dir}/g4-wbench.tar.gz
if [ -f "${g4w_target}" ]; then
  echo "ERROR: file ${g4w_target} exists already: cannot overwrite"
else
  testrunn_dir=css-testrunner-data
  if [ -d "${testrunn_dir}" ]; then
    tar czf ${g4w_target} g4-wbench ${testrunn_dir} team-wkspace .showho .bashrc .g4wks_bashrc
  else
    tar czf ${g4w_target} g4-wbench team-wkspace .showho .bashrc .g4wks_bashrc
  fi
  if [ -f "${g4w_target}" ]; then
    echo "The ${g4w_target} file has been created successfully"
    ls -lart ${g4w_target}
  else
    echo "ERROR: ${g4w_target} creation command succeeded, but the file seems not being there"
    exit 1
  fi
fi

cd - >/dev/null 2>&1
