
printA()
{
  echo "0;0;0;0"
  echo "0;0;0;0"
  echo "0;0;0;0"
  echo "0;1;0;2.25972"
}


read -r tot_wk_in tot_wk_out tot_wk_ein tot_wk_eout <<< $(printA | awk -F";" 'BEGIN {tot_in=0; tot_out=0; tot_ein=0; tot_eout=0} {tot_in=tot_in+0+$1; tot_out=tot_out+0+$2; tot_ein=tot_ein+0+$3; tot_eout=tot_eout+0+$4} END {print tot_in " " tot_out " " tot_ein " " tot_eout}')

echo $tot_wk_in
echo $tot_wk_out
echo $tot_wk_ein
echo $tot_wk_eout
