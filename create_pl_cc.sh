#!/bin/bash

#
echo "This is HIGHLY impactful on the environment."
read -p "Are you sure you want to continue? (y/n): " choice
case "${choice}" in
  y|Y ) echo "Proceeding...";;
  n|N ) echo "Aborting."; exit 0;;
  * ) echo "Invalid input. Aborting."; exit 1;;
esac

cur_dir=$(pwd)

SRC_DIR_TMPL=${cur_dir}/src_tmpl
SRC_DIR=${cur_dir}/src

#OUT_FILE=${SRC_DIR}/TESTPL.CC
OUT_FILE=${SRC_DIR}/PhysicsManager.cc

#head
echo "Generating ${OUT_FILE}: HEAD first ..."
cat ${SRC_DIR_TMPL}/PhysicsManager_1.pre_cc > ${OUT_FILE}
echo "Done."
echo "Adding 'include' statements ..."
#includes
inc_cfg=${SRC_DIR_TMPL}/physics_lists_build.txt
num_inc=$(cat "${inc_cfg}" | grep -v "#" | wc -l)
echo "'include' statements to add: ${num_inc}"
idx=1
while [ $idx -le $num_inc ]; do
    #cur_i_line=$(tail -$idx "${inc_cfg}" | tail -1)
    #echo "$idx - $cur_i_line"
    read -r hh_file ph_string <<< $(cat "${inc_cfg}" | grep -v "#" | head -$idx | tail -1 | awk -F, '{ print$1 " " $2 }')
    echo "   Adding 'include' nbr. $idx - hh file: ${hh_file}, ph.: ${ph_string}"
    echo "#include \"${hh_file}\"" >> ${OUT_FILE}
    let idx=$idx+1
done
echo "Done."

#init
echo "Adding initialization ..."
cat ${SRC_DIR_TMPL}/PhysicsManager_2.pre_cc >>${OUT_FILE}
echo "Done."

#loop
echo "Adding loop items (${num_inc}) ..."
idy=1
while [ $idy -le $num_inc ]; do
    read -r ph_string ph_class <<< $(cat "${inc_cfg}" | grep -v "#" | head -$idy | tail -1 | awk -F, '{ print$2 " " $3 }')
    echo "   Adding loop item nbr. $idy - ph.: ${ph_string}, ph. class: ${ph_class}"
    if_pfix=""
    if [ $idy -gt 1 ]; then
      if_pfix="} else "
    fi
    cat ${SRC_DIR_TMPL}/PhysicsManager_3.pre_cc | sed "s/__TMPL_IF_PFIX_/${if_pfix}/g" | sed "s/__TMPL_PHYS_STR_/${ph_string}/g" | sed "s/__TMPL_PHYS_CLASS_/${ph_class}/g" >>${OUT_FILE}
    let idy=$idy+1
done
echo "Done."

#tail
echo "Adding tail ..."
cat ${SRC_DIR_TMPL}/PhysicsManager_4.pre_cc >>${OUT_FILE}
echo "Done. The requested source file has been created successfully."
echo
echo "diff generated vs. good results:"
diff ${OUT_FILE} ${SRC_DIR_TMPL}/PhysicsManager.cc.GOOD250504
echo
#cat ${OUT_FILE}
