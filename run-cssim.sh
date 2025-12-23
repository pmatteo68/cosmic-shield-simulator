#!/bin/bash

#################### EDITABLE SECTION - BEGIN ###########

#Careful with ON - uses lots of disk space
sav_stepdata=ON

#Careful: consumes much CPU
eval_radiations=ON

#If stepdata are saved, this is the csv implementation.
#Can be G4 or custom. With G4, there is one csv per thread.
#With custom, one single file (simulation performance may be affected, but at the end no need to merge)
#stepd_csv_impl=custom
stepd_csv_impl=G4

# Layers you do NOT want to count, in the weight evaluation
wgt_exclusions=Detector,detector

#################### EDITABLE SECTION - END #############

# This function helps in distinguishing the case when this script is invoked directly vs from the optimizer
setVariableRunPaths()
{
  local runId=$1
  local geomConfDir=$2
  local geomConfFileName=$3
  run_id=${runId}
  geom_filename=${geomConfFileName}
  geom_file=${geomConfDir}/${geom_filename}
  log_file_name=css-${run_id}.log
  log_file=${logs_dir}/${log_file_name}
  out_dir=${out_dir_root}/r${run_id}
}

# If this script is called by the optimizer, and if CSS_OPT_SAV_STEPDATA is set in the optimization launcher script, its value prevails
if [ "x${CSS_OPT_SAV_STEPDATA}" == "x" ]; then
  echo "Local sav_stepdata setting applies"
else
  sav_stepdata=${CSS_OPT_SAV_STEPDATA}
  echo "sav_stepdata setting IS BEING OVERRIDDEN TO: ${CSS_OPT_SAV_STEPDATA}"
fi

op_mode=$1
ui_mode=ui
batch_mode=batch
if [ "x${op_mode}" == "x" ]; then
  echo "Argument missing: opmode (it can be '${batch_mode}' or '${ui_mode}')"
  exit 1
else
  if [ "x${op_mode}" != "x${ui_mode}" ] && [ "x${op_mode}" != "x${batch_mode}" ]; then
    echo "Unknown opmode (found: $op_mode, must be '${ui_mode}' or '${batch_mode}'.)" >&2
    exit 1
  fi
fi

script_dir="$(dirname "$(readlink -f "$0")")"
cd "${script_dir}"
curr_dir=$(pwd)
proj_dir=${curr_dir}

logs_dir=${proj_dir}/logs
out_dir_root=${proj_dir}/out

conf_dir=${proj_dir}/config
conf_dir_optim=${conf_dir}/optim
geom_basefilename=geometry
geom_baseextns=json
macros_dir=${proj_dir}/macros

app_cfg_file=${conf_dir}/appconfig.json
cust_mat_file=${conf_dir}/custom-materials.json
beam_file=${conf_dir}/beam.json
cust_part_file=${conf_dir}/custom-particles.json
wr_res_file=${conf_dir}/wrresolver.json
mac_file=${macros_dir}/run.mac
initui_mac_file=${macros_dir}/init_vis.mac

curr_tstamp=$(date +"%y%m%d%H%M%S")
#DIRECT CALL MODE: run_id, geom_filename, geom_file, log_file_name, log_file, out_dir will be set with this call
setVariableRunPaths ${curr_tstamp} ${conf_dir} ${geom_basefilename}.${geom_baseextns}

if [ "x${op_mode}" == "x${batch_mode}" ]; then
  optim_run_id=$2
  if [ "x${optim_run_id}" = "x" ]; then
    RUN_IN_BACKGROUND=true
  else
    #If run id is passed on command line, then it is the optimizer which launched this script, and execution has NOT to be in background.
    #Also, in this case the geometry configuration file varies.
    RUN_IN_BACKGROUND=false
    #OPTIMIZER CALL MODE: run_id, geom_filename, geom_file, log_file_name, log_file, out_dir will be set with this call
    setVariableRunPaths ${optim_run_id} ${conf_dir_optim} ${geom_basefilename}_${optim_run_id}.${geom_baseextns}
  fi
  sync_cmd=(
    "${curr_dir}/bin/xrun-cssim-single.sh"
    -om batch
    -ac "${app_cfg_file}"
    -cm "${cust_mat_file}"
    -ge "${geom_file}"
    -cp "${cust_part_file}"
    -be "${beam_file}"
    -wr "${wr_res_file}"
    -bm "${mac_file}"
    -so "${out_dir}"
    -sd "${sav_stepdata}"
    -ci "${stepd_csv_impl}"
    -er "${eval_radiations}"
    -we "${wgt_exclusions}"
    -rt "${run_id}"
    -sl "${log_file}"
  )
  if [ "x${RUN_IN_BACKGROUND}" = "xtrue" ]; then
    echo
    echo "===================================================="
    echo "Cosmic Shield Simulator - Single run execution shell"
    echo "===================================================="
    nohup "${sync_cmd[@]}" >/dev/null 2>&1 &
    ppid=$!
    echo "Simulator process launched in background (opmode: ${op_mode}, pid: ${ppid})"
    echo
    echo "At the end, check out the following directory for results:"
    echo "-> ${out_dir}"
    echo
    echo "Check log with:"
    echo "tail -f ${log_file}"
    echo
  else
    echo "[$(date)] Cosmic Shield Simulator process single run launched synchronously (geom.: ${geom_filename}, log: ${log_file}). Waiting for completion..."
    "${sync_cmd[@]}" >/dev/null 2>&1

    #echo "[$(date)] [DEBUG] command being executed (from: $(pwd)): mv ${geom_file} ${out_dir}/"
    mv ${geom_file} ${out_dir}/
    echo "[$(date)] Cosmic Shield Simulator process completed (results: ${out_dir})"

    num_layers=na
    geom_as_str=na
    geom_file_to_check=${out_dir}/${geom_filename}
    if [ -f "${geom_file_to_check}" ]; then
      num_layers=$(jq '.layers | length' ${geom_file_to_check})
      geom_as_str=$(jq -r '[.layers[] | "\(.material) (\(.thickness))"] | join(", ")' ${geom_file_to_check})
    fi

    kpis_as_str=na
    kpis_file=${out_dir}/glob_kpis_${run_id}.csv
    if [ -f "${kpis_file}" ]; then
      #echo "KPIs:"
      ###cat "${kpis_file}" | awk -F';' 'NR==1 {for (i=1; i<=NF; i++) { sub(/\r$/, "", $i); header[i]=$i }} NR==2 {for (j=1; j<=NF; j++) print "  " header[j] ": " $j}'
      ##kpis_as_str=$(cat "${kpis_file}" | awk -F';' 'NR==1 {for (i=1; i<=NF; i++) { sub(/\r$/, "", $i); header[i]=$i }} NR==2 {for (j=1; j<=NF; j++) print "" header[j] ": " $j}' | awk 'BEGIN{ORS=""} {printf "%s%s", sep, $0; sep=", "} END {printf "\n"}')
      #kpis_as_str=$(cat "${kpis_file}" | awk -F';' 'NR==1 {for (i=1; i<=NF; i++) { sub(/\r$/, "", $i); header[i]=$i }} NR==2 {for (j=1; j<=NF; j++) print "" header[j] ": " $j}' | awk 'BEGIN{ORS=""} {printf "%s%s", sep, $0; sep=", "}')
      kpis_as_str=$(cat "${kpis_file}" | awk -F';' 'NR==1 {for (i=1; i<=NF; i++) { sub(/\r$/, "", $i); header[i]=$i }} NR==2 {for (j=1; j<=NF; j++) { sub(/\r$/, "", $j); print "" header[j] ": " $j }}' | awk 'BEGIN{ORS=""} {printf "%s%s", sep, $0; sep=", "}')
    fi
    #echo "[$(date)][simtraceDEBUG] -- ${kpis_as_str} --"
    echo "[$(date)][simtrace][${run_id}] GEOMETRY (layers: ${num_layers}): [${geom_as_str}], KPIs: [${kpis_as_str}]"

  fi
else
  echo "Cosmic Shield Simulator process is being launched interactively (opmode: ${op_mode})"
  echo "Log: ${log_file}"
  ${curr_dir}/bin/xrun-cssim-single.sh \
    -om ui \
    -ac ${app_cfg_file} \
    -cm ${cust_mat_file} \
    -ge ${geom_file} \
    -cp ${cust_part_file} \
    -be ${beam_file} \
    -wr ${wr_res_file} \
    -bm ${mac_file} \
    -um ${initui_mac_file} \
    -so ${out_dir} \
    -sd ${sav_stepdata} \
    -ci ${stepd_csv_impl} \
    -er ${eval_radiations} \
    -we ${wgt_exclusions} \
    -rt ${run_id} \
    -sl ${log_file}
fi

cd - >/dev/null 2>&1

