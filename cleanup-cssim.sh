#!/bin/bash

. ./bin/env-cssim.sh

echo "Cleaning up (${BUILD_DIR}) ..."
rm -rf ${BUILD_DIR} 2>/dev/null
echo "Cleaned up (removed: ${BUILD_DIR})."

echo

echo "Cleaning temp files (logs/*.log, out/r*, temp/*.csv, config/optim/*.json) ..."
rm logs/*.log
rm -r out/r*
rm config/optim/*.json
rm temp/*.csv

echo "Done."
