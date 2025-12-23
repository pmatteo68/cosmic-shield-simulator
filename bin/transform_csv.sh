#!/bin/bash

# Parse options
while [[ $# -gt 0 ]]; do
  case "$1" in
    -f|--file) input_file="$2"; shift 2 ;;
    -i|--isep) input_sep="$2"; shift 2 ;;
    -o|--osep) output_sep="$2"; shift 2 ;;
    -h|--head) header_needed="$2"; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done


# Validate input file
[[ -z "$input_file" || ! -f "$input_file" ]] && {
  echo "Usage: $0 -f <input_file> -i <input_separator> -o <output_separator> -h <header needed (true/false)>"
  exit 1
}

# The separator in the file is assigned
raw_sep="${input_sep}"

columns=()

# First ascertain if it's a custom file (header in first row)
line_num=0
cust_header_line_idx=0
while IFS= read -r line; do
  let line_num=${line_num}+1
  [[ "$line" =~ ^# ]] && continue  # Skip comment lines
  # Try splitting with ${output_sep}
  IFS="${output_sep}" read -ra fields <<< "$line"
  if (( ${#fields[@]} < 2 )); then
    # Try ${input_sep} if output_sep failed
    IFS="${input_sep}" read -ra fields <<< "$line"
  else
    # as with output_sep we succeeded, means that the input sep is ALREADY the output sep
    input_sep=${output_sep}
    raw_sep="${input_sep}"
  fi
  columns=("${fields[@]}")
  cust_header_line_idx=$line_num
  break
done < "${input_file}"
num_fields=${#columns[@]}

if [ ${num_fields} -lt 1 ]; then
  # (if file was not custom) Read through the header lines to parse columns
  #echo "Reading header and determining columns..."
  cust_header_line_idx=0
  while IFS= read -r line; do
    #echo "Processing line: $line"
 
    # Gather columns from the header
    if [[ $line == "#column "* ]]; then
      columns+=("$(echo "$line" | awk '{print $3}')")
      #echo "Found column: $(echo "$line" | awk '{print $3}')"
    fi

    # Exit once we've processed the metadata
    if [[ $line != \#* ]]; then
      break
    fi
  done < "$input_file"
  num_fields=${#columns[@]}
fi

if [ "x${header_needed}" == "xtrue" ]; then
  # Output the columns header in the chosen separator
  #echo "Output header:"
  (IFS="$output_sep"; echo "${columns[*]}")
fi

# Process the data lines, replace separator
#echo "Processing data lines..."
let tail_start_line=1+${cust_header_line_idx}
tail -n +${tail_start_line} "$input_file" | grep -v '^#' | tr -d '\r' | awk -F"$raw_sep" -v OFS="$output_sep" -v n="${#columns[@]}" '
NF >= n {
  for (i = 1; i <= n; i++) {
    printf "%s", $i
    if (i < n) printf OFS
  }
  print ""
}'

