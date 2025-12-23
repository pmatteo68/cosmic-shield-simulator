#!/bin/bash

#################### EDITABLE SECTION - BEGIN ###########

#Base output file name
export CSS_G4_MATERIALS_BPATH=../proj/g4_materials

#if the following file containing custom materials definitions exists, such materials will be added.
add_mats_config=../proj/config/custom-materials.json

#################### EDITABLE SECTION - END #############

echo "[$(date)] Dumping the Geant4 materials database (target: ${CSS_G4_MATERIALS_BPATH})"

if [ -f ${add_mats_config} ]; then

  echo "[$(date)] Custom materials definitions found (source: ${add_mats_config})"
  cust_json_target="${CSS_G4_MATERIALS_BPATH}_custom.json"
  echo "[$(date)] Dumping custom materials to JSON format (target: ${cust_json_target})"
  cat ${add_mats_config} | jq '{materials: [.materials[] | {matname: .name, matdensity: .density}]}' > ${cust_json_target}

  cust_txt_target="${CSS_G4_MATERIALS_BPATH}_custom.txt"
  echo "[$(date)] Dumping custom materials to TXT format (target: ${cust_txt_target})"
  cat ${add_mats_config} | jq -r '.materials[].name' > ${cust_txt_target}
else
  echo "[$(date)] Custom materials configuration file (${add_mats_config}) was not found, and will not be processed."
fi

echo "[$(date)] Dumping the Geant4-native materials"
echo "[$(date)] *** This is a simulator run, but it will NOT launch any true simulation. A request to dump the G4 materials database has been made by configuration, and in this case the program will exit RIGHT AFTER this has been done. This is expected behavior, as the G4 materials database dumping functionality inflates memory in a non-recoverable way. ***"

./run-cssim.sh batch

