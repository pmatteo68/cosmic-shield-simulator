#!/bin/bash

if [[ $# -ne 1 ]]; then
  echo >&2
  echo "Usage: $0 <path>" >&2
  echo >&2
  exit 1
fi

dir_name=$1

if [ -d "${dir_name}" ]; then
  find "${dir_name}" -mindepth 1 -maxdepth 1 -type d | grep "test\-"
else
  echo "The specified directory (${dir_name}) does not exist" >&2
fi

