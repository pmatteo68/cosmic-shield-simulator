#!/bin/bash

. ./bin/g4-csv-env.sh

countFileRecords() {
  filename=$1
  echo $(cat $filename 2>/dev/null | grep -v "#" | wc -l)
}

makeAbsPath()
{
  inPath=$1
  rootPath=$2
  if [[ "${inPath}" = /* ]]; then
    #absolute - unchanged
    echo "${inPath}"
  else
    #relative - prepending what needed
    echo "${rootPath}/${inPath}"
  fi
}

usage()
{
  echo
  echo "Usage type #1: $0 -r <run timestamp>"
  echo "(and the run timestamp is supposed to correspond to an existing directory under ${out_subdir} subdirectory)"
  echo
  echo "Example:"
  echo "$0 -r 250611234323111"
  echo
  #echo "Usage: $0 -r <run timestamp> -is <input separator> -os <output separator>"
  echo "Usage type #2: $0 -i <input dir.> -o <output dir.>"
  echo "(where input dir. contains the CSV files to merge, and output dir. is the target directory for the output)"
  echo
  echo "Example:"
  echo "$0 -i /home/user1/test1/out -o /home/user1/mergedresults"
  echo
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -r|--run) run_tstamp="$2"; shift 2 ;;
    -i|--in-files) input_dir="$2"; shift 2 ;;
    -o|--out-files) output_dir="$2"; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
  #case "$1" in
  #  -r|--run) run_tstamp="$2"; shift 2 ;;
  #  -is|--isep) in_sep="$2"; shift 2 ;;
  #  -os|--osep) out_sep="$2"; shift 2 ;;
  #  *) echo "Unknown option: $1"; exit 1 ;;
  #esac
done

curr_dir=$(pwd)
out_subdir=${curr_dir}/out

in_sep=","
out_sep=";"

# Validate input dir
if [ ! "x${run_tstamp}" == "x" ]; then
  #old invocation style
  raw_data_dir=${out_subdir}/r${run_tstamp}
  if [ -d "${raw_data_dir}" ]; then
    input_dir=${raw_data_dir}
    output_dir=${input_dir}
    out_file=${output_dir}/merged_data_${run_tstamp}.csv
  else
    usage
    exit 1
  fi
else
  #new invocation style
  if [[ -n "${input_dir}" && -n "${output_dir}" && -d "${input_dir}" && -d "${output_dir}" ]]; then
    input_dir=$(makeAbsPath "${input_dir}" "${curr_dir}")
    output_dir=$(makeAbsPath "${output_dir}" "${curr_dir}")
    out_file=${output_dir}/merged_data.csv
  else
    usage
    exit 1
  fi
fi

echo "============================================="
echo "Cosmic Shield Simulator - CSV Merging Utility"
echo "v. 1.3.0 - M. Picciau"
echo "============================================="

listFilesByFilePattern()
{
  fpattern=$1
  ls $fpattern 2>/dev/null
}

>${out_file}
echo
echo "[$(date)] Input directory: ${input_dir}"
echo "[$(date)] Target output file: ${out_file}"
raw_stepdata_fpattern=$(./bin/printRawStepDataFPatt.sh)
echo "[$(date)] Raw data filename pattern: ${raw_stepdata_fpattern}"
cd ${input_dir}

#echo "listFilesByFilePattern $raw_stepdata_fpattern"
#listFilesByFilePattern $raw_stepdata_fpattern
nfiles=$(listFilesByFilePattern "$raw_stepdata_fpattern" | wc -l)
echo "[$(date)] Num. raw files: ${nfiles}"

fidx=0
exp_recs=0
for cur_rf in $(listFilesByFilePattern "$raw_stepdata_fpattern"); do
  let fidx=${fidx}+1
  is_curfile_custom=$(${curr_dir}/bin/is_stepdata_csv_cust.sh -f "${cur_rf}" -i "${in_sep}" -o "${out_sep}")
  curr_recs=$(countFileRecords ${cur_rf})
  if [ "x${is_curfile_custom}" == "xtrue" ]; then
    #if the stepdata csv is the custom one, then the header line needs to be subtracted
    let curr_recs=${curr_recs}-1
  fi
  let exp_recs=${exp_recs}+${curr_recs}
  echo "[$(date)] Processing file[${fidx}]: ${cur_rf} (records: ${curr_recs})"
  head_needed=false
  if [ $fidx -eq 1 ]; then
    head_needed=true
  fi
  #echo "${curr_dir}/bin/transform_csv.sh -f ${cur_rf} -i ${in_sep} -o ${out_sep} -h ${head_needed}"
  ${curr_dir}/bin/transform_csv.sh -f "${cur_rf}" -i "${in_sep}" -o "${out_sep}" -h "${head_needed}" >> ${out_file}
  #because of header we subtract one
  let merged_out_recs=$(countFileRecords ${out_file})-1
done
if [ $nfiles -gt 0 ]; then
  echo "[$(date)] Output file created successfully:"
  echo
  echo "  -> ${out_file}"
  echo "  (records: ${merged_out_recs}, expected: ${exp_recs})"
else
  echo "[$(date)] No processing takes place as NO files to merge was found in ${input_dir}."
fi
echo

cd - >/dev/null 2>&1

