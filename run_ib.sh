#!/bin/bash
# run_ib.sh : IB electron cut check, LD2 18335, CxC 18344, CuSn 18384, first 50 DST files each.
# Run from the clas-framework folder.
#   electron_analysis  (config/ib_fig_<T>.toml, PCAL and DC cuts off) -> root/ib_fig_<T>.root     figures
#   electron_selection (config/table5_<T>_IB.toml, all cuts on)       -> root/table5_<T>_IB.root  Table 5 (cutflow in logs/)
set -u
OUT=/volatile/clas12/mtenorio/pi0/ib_check
[ -x build/analyses/electron_analysis ] || { echo "run this from the clas-framework folder"; exit 1; }
declare -A RUN=([LD2]=018335 [CxC]=018344 [CuSn]=018384)
for t in LD2 CxC CuSn; do
  ls -d /cache/clas12/rg-d/production/pass1/recon/$t/dst/recon/${RUN[$t]}/*.hipo | head -50 > $OUT/logs/files_$t.txt
  files=$(paste -sd, $OUT/logs/files_$t.txt)
  echo "=== $(date '+%F %T')  $t run ${RUN[$t]}: $(wc -l < $OUT/logs/files_$t.txt) files"
  ./build/analyses/electron_analysis --config config/ib_fig_$t.toml --input "$files" --output $OUT/root/ib_fig_$t.root > $OUT/logs/ib_fig_$t.log 2>&1
  echo "    electron_analysis  exit code $?"
  ./build/analyses/electron_selection --config config/table5_${t}_IB.toml --input "$files" --output $OUT/root/table5_${t}_IB.root > $OUT/logs/table5_${t}_IB.log 2>&1
  echo "    electron_selection exit code $?"
done
echo "=== $(date '+%F %T')  ALL DONE"
