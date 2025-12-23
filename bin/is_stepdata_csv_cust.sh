#!/bin/bash

# Parse options
while [[ $# -gt 0 ]]; do
  case "$1" in
    -f|--file) input_file="$2"; shift 2 ;;
    -i|--isep) input_sep="$2"; shift 2 ;;
    -o|--osep) output_sep="$2"; shift 2 ;;
    *) echo "Unknown option: $1"; exit 1 ;;
  esac
done


# Validate input file
[[ -z "$input_file" || ! -f "$input_file" ]] && {
  echo "Usage: $0 -f <input_file> -i <input_separator> -o <output_separator>"
  exit 1
}

columns=()

# Ascertain if it's a custom file (header in first row)
#line_num=0
#cust_header_line_idx=0
while IFS= read -r line; do
  #let line_num=${line_num}+1
  [[ "$line" =~ ^# ]] && continue  # Skip comment lines
  # Try splitting with ${output_sep}
  IFS="${output_sep}" read -ra fields <<< "$line"
  if (( ${#fields[@]} < 2 )); then
    # Try ${input_sep} if output_sep failed
    IFS="${input_sep}" read -ra fields <<< "$line"
  fi
  columns=("${fields[@]}")
  #cust_header_line_idx=$line_num
  break
done < "${input_file}"
num_fields=${#columns[@]}

if [ ${num_fields} -lt 1 ]; then
  echo "false"
else
  echo "true"
fi

