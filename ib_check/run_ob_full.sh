#!/bin/bash
# run_ob_full.sh : OB electron cut check, ALL DST files of LD2 18543, CxC 18465, CuSn 18591.
# Run from the clas-framework folder. Waits until run_ib_full.sh has printed ALL DONE.
#   electron_analysis  (config/ob_fig_<T>.toml, PCAL and DC cuts off) -> root_full/ob_fig_<T>.root     figures
#   electron_selection (config/table5_<T>_OB.toml, all cuts on)       -> root_full/table5_<T>_OB.root  Table 5 (cutflow in logs_full/)
set -u
OUT=/volatile/clas12/mtenorio/pi0/ib_check
[ -x build/analyses/electron_analysis ] || { echo "run this from the clas-framework folder"; exit 1; }
echo "=== $(date '+%F %T')  waiting for the IB run to finish"
until grep -q "ALL DONE" $OUT/logs_full/run_ib_full.log 2>/dev/null; do sleep 120; done
declare -A RUN=([LD2]=018543 [CxC]=018465 [CuSn]=018591)
for t in LD2 CxC CuSn; do
  ls -d /cache/clas12/rg-d/production/pass1/recon/$t/dst/recon/${RUN[$t]}/*.hipo > $OUT/logs_full/files_${t}_OB.txt
  files=$(paste -sd, $OUT/logs_full/files_${t}_OB.txt)
  echo "=== $(date '+%F %T')  $t run ${RUN[$t]}: $(wc -l < $OUT/logs_full/files_${t}_OB.txt) files"
  ./build/analyses/electron_analysis --config config/ob_fig_$t.toml --input "$files" --output $OUT/root_full/ob_fig_$t.root > $OUT/logs_full/ob_fig_$t.log 2>&1
  echo "    electron_analysis  exit code $?"
  ./build/analyses/electron_selection --config config/table5_${t}_OB.toml --input "$files" --output $OUT/root_full/table5_${t}_OB.root > $OUT/logs_full/table5_${t}_OB.log 2>&1
  echo "    electron_selection exit code $?"
done
echo "=== $(date '+%F %T')  ALL DONE"
