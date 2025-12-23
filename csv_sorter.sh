#!/bin/bash

countFileRecords() {
  filename=$1
  skip_lines=$2
  echo $(tail -n +"$((${skip_lines} + 1))" "$filename" | grep -v '^[[:space:]]*#' | grep -v '^[[:space:]]*$' | wc -l)
  #echo $(cat $filename | grep -v "#" | wc -l)
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -i|--ifile) input_file="$2"; shift 2 ;;
    -o|--ofile) out_file="$2"; shift 2 ;;
    -s|--sortBy) sort_by="$2"; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done

echo
echo "============================================="
echo "Cosmic Shield Simulator - CSV Sorting Utility"
echo "v. 1.0.0 - M. Picciau"
echo "============================================="

# assumes input file with a header row, and with ";" as separator

curr_dir=$(pwd)
sortBy_sep=","
in_sep=";"
#out_sep=";"

tmp_dir=${curr_dir}/temp
mkdir ${tmp_dir} 2>/dev/null

# Validate input file
if [ -f "${input_file}" ]; then
  >${out_file}
else
  echo "Usage: $0 -i <input file> -o <output file> -s <sort by>"
  echo
  echo "Example:"
  echo "$0 -i ./out/mycsv.csv -o ./out/mynewcsv.csv -s EventId,TrackId,StepNbr"
  exit 1
fi

echo
echo "Input file: ${input_file}"
echo "Target output file: ${out_file}"
echo "Sort by: ${sort_by}"
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Processing: BEGIN"

# Read header to get the field names
#header_row=$(head -n 1 "${input_file}")
#header_row=$(grep -v '^[[:space:]]*#' "${input_file}" | head -n 1)
#read -r header_line_num header_row < <(awk 'BEGIN{FS=OFS="${in_sep}"} /^[[:space:]]*#/ || /^[[:space:]]*$/ {next} {print NR, $0; exit}' "${input_file}")
IFS= read -r header_info < <(awk '/^[[:space:]]*#/ || /^[[:space:]]*$/ {next} {print NR ";" $0; exit}' "${input_file}")
header_line_num="${header_info%%;*}"
header_row="${header_info#*;}"

echo "Header found at line: ${header_line_num}. Counting input records ..."
in_records=$(countFileRecords ${input_file} ${header_line_num})
echo "Done. Input records: ${in_records}"
#echo "Header: $header_row"

# Split the sorting field names into an array
IFS="${sortBy_sep}" read -ra sort_fields <<< "$sort_by"

# Map field names to column numbers (based on header)
declare -A field_map
IFS="${in_sep}" read -ra header_fields <<< "${header_row}"
for i in "${!header_fields[@]}"; do
    field_map["${header_fields[$i]}"]=$((i + 1))  # Store column number (1-based)
done

# infer col types
detection_file_sz=500
sample_sz=100
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Detecting sorting fields types (sample sz: ${sample_sz}, detection file size: ${detection_file_sz}) ..."
tmp_eval_in_file=${tmp_dir}/tmp_csv_sort_$(date +"%y%m%d%H%M%S%3N").csv
head -${detection_file_sz} "${input_file}" > "${tmp_eval_in_file}"
#echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Detection file built (${tmp_eval_in_file})"
declare -A is_numeric_col
for field in "${sort_fields[@]}"; do
    field_index=${field_map[$field]}
    if [[ -n "${field_index}" ]]; then
        #echo "[DEBUG]  Inferring type for: $field"
        #echo "[DEBUG]     column_index($field}): ${field_map[$field]}"
        #numeric_count=$(awk -F"${in_sep}" -v vcol="${field_index}" -v vsample_sz=${sample_sz} 'NR>1 && NR<=vsample_sz+1 {print $vcol}' "${input_file}" | grep -E '^[+-]?[0-9]+([.][0-9]+)?([eE][+-]?[0-9]+)?$' | wc -l)
        numeric_count=$(tail -n +"$((${header_line_num} + 1))" "${tmp_eval_in_file}" | grep -v '^[[:space:]]*#' | grep -v '^[[:space:]]*$' | awk -F"${in_sep}" -v vcol="${field_index}" -v vsample_sz=${sample_sz} 'NR<=vsample_sz {print $(vcol)}' | grep -E '^[+-]?[0-9]+([.][0-9]+)?([eE][+-]?[0-9]+)?$' | wc -l)
        #echo "[DEBUG]     numeric_count=${numeric_count}"
        if [ ${numeric_count} -eq ${sample_sz} ]; then
            is_numeric_col["${field_index}"]=1
            echo "    [${field_index}] ${field}: numeric"
        else
            is_numeric_col["${field_index}"]=0
            echo "    [${field_index}] ${field}: non numeric"
        fi
    else
        echo "Error: Field '$field' not found in header!"
        #echo "[DEBUG]  Removing temporary evaluation file (1)"
        rm ${tmp_eval_in_file} 2>&1
        exit 1
    fi
done
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Done."
#echo "[DEBUG]  Removing temporary evaluation file (2)"
rm ${tmp_eval_in_file} 2>&1

# Construct sort options based on the field mapping
sort_args=()
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Constructing sort options based on the field mapping ..."
for field in "${sort_fields[@]}"; do
    field_index=${field_map[$field]}
    common_sort_cond="-k${field_index},${field_index}"
    if [[ ${is_numeric_col["${field_map[$field]}"]} -eq 1 ]]; then
        sort_args+=("${common_sort_cond}n") # Use numeric sorting
    else
        sort_args+=("${common_sort_cond}")  # Use lexigographical sorting
    fi
done
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Done."

# Use sort to sort the file based on the specified columns and then append the header
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Sorting ..."
echo "$header_row" > "${out_file}"
tail -n +2 "${input_file}" | sort "${sort_args[@]}" -t"${in_sep}" >> "${out_file}"
sorted_out_recs=$(countFileRecords ${out_file} 1)
echo "[$(date +"%y-%m-%d %H:%M:%S.%3N")] Done. Processing: END."

echo "Output file created successfully:"
echo
echo "-> ${out_file}"
echo "(records: ${sorted_out_recs}, expected: ${in_records})"
echo
#cd - >/dev/null 2>&1
