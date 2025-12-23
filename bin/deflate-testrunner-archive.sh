#!/bin/bash

# Check if exactly two arguments are provided
if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <testrunner data dir> <archive path>" >&2
  exit 1
fi

tr_data_dir="$1"
arc_path="$2"

if [[ ! -d "${tr_data_dir}" ]]; then
  echo "Test runner data directory (${tr_data_dir}) not found" >&2
  exit 1
fi
if [[ ! -f "${arc_path}" ]]; then
  echo "Test runner archive (${arc_path}) not found" >&2
  exit 1
fi

filename=$(basename -- "${arc_path}")
case "$filename" in
  *.zip|*.tar|*.gz|*.tgz|*.tar.gz)
    # Proceed
    ;;
  *)
    echo "Test runner archive (${arc_path}) is not of the expected type" >&2
    exit 1
    ;;
esac

# Create a timestamped directory
timestamp=$(date +"%y%m%d%H%M%S")
dest_dir="${tr_data_dir}/s$timestamp"
mkdir -p "$dest_dir" || exit 1

# Copy the file into the directory
cp "${arc_path}" "$dest_dir/" || exit 1
cd "$dest_dir" || exit 1

# Unpack based on file type
case "$filename" in
  *.tar.gz|*.tgz) tar -xzf "$filename" ;;
  *.tar) tar -xf "$filename" ;;
  *.gz) gunzip "$filename" ;;
  *.zip) unzip "$filename" ;;
esac

#remove archive
rm "${filename}" 2>/dev/null

# Find 'test-' dir and print its parent
#test_path=$(find . -type d -name "common" | head -n 1)
test_path=$(find . -type d -name "test-*" | head -n 1)
if [ -n "$test_path" ]; then
  parent_dir=$(dirname "$test_path")
  mkdir "${parent_dir}/logs" 2>/dev/null
  #echo "Parent of 'test-' dir: $parent_dir"
  echo "$(pwd)/${parent_dir}"
fi

cd - >/dev/null 2>&1
