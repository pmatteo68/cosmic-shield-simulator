#!/bin/bash

. ../../../../scripts/bin/g4env-s3.sh
. ./bin/env-cssim.sh

echo "Cosmic Shield Simulator v. ${CSS_VER} build shell"

#echo "Setting Geant environment ..."
#. ${G4_INSTALL_ROOT}/bin/geant4.sh

echo "Re-creating build directory (${BUILD_DIR}) ..."
rm -rf ${BUILD_DIR} 2>/dev/null
mkdir ${BUILD_DIR}
echo "Done."

PROJ_DIR=proj
JS_FILENAME=json.hpp
JS_FOLDER=include/nlohmann

if [ -f ${JS_FOLDER}/${JS_FILENAME} ]; then
  echo "${JS_FILENAME} found (${JS_FOLDER}/${JS_FILENAME})"
else
  echo "Downloading ${JS_FILENAME} ..."
  wget https://github.com/nlohmann/json/releases/latest/download/${JS_FILENAME}
  echo "Done."
  mkdir -p ${JS_FOLDER} 2>/dev/null
  mv ${JS_FILENAME} ${JS_FOLDER}/
fi

echo "Moved in position ($(pwd))"
cd ${BUILD_DIR}
echo "Building (cmake) ..."
cmake -DGeant4_DIR=${G4_CONFIG_FOLDER} \
      -DXercesC_INCLUDE_DIR=${XERCES_INSTALL_ROOT}/include \
      -DXercesC_LIBRARY=${XERCES_SO_LIB} \
      ../${PROJ_DIR}
echo "Done. Building (make) ..."
make -j${G4_NPROC}
echo "Build completed."

