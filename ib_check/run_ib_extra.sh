#!/bin/bash
# run_ib_extra.sh : IB electron cut check on two extra IB runs, ALL DST files: LD2 18305 (first IB run, rgd_v1.0_30kHz
# trigger) and CxC 18401 (last IB run, closest in time to the OB period). Same configs and programs as run_ib_full.sh.
# Run from the clas-framework folder.
#   electron_analysis  (config/ib_fig_<T>.toml, PCAL and DC cuts off) -> root_full/ib_fig_<T>_<run>.root     figures
#   electron_selection (config/table5_<T>_IB.toml, all cuts on)       -> root_full/table5_<T>_IB_<run>.root  Table 5 (cutflow in logs_full/)
set -u
OUT=/volatile/clas12/mtenorio/pi0/ib_check
[ -x build/analyses/electron_analysis ] || { echo "run this from the clas-framework folder"; exit 1; }
for tr in CuSn:18379; do
  t=${tr%%:*}; r=${tr##*:}
  ls -d /cache/clas12/rg-d/production/pass1/recon/$t/dst/recon/0$r/*.hipo > $OUT/logs_full/files_${t}_$r.txt
  files=$(paste -sd, $OUT/logs_full/files_${t}_$r.txt)
  echo "=== $(date '+%F %T')  $t run $r: $(wc -l < $OUT/logs_full/files_${t}_$r.txt) files"
  ./build/analyses/electron_analysis --config config/ib_fig_$t.toml --input "$files" --output $OUT/root_full/ib_fig_${t}_$r.root > $OUT/logs_full/ib_fig_${t}_$r.log 2>&1
  echo "    electron_analysis  exit code $?"
  ./build/analyses/electron_selection --config config/table5_${t}_IB.toml --input "$files" --output $OUT/root_full/table5_${t}_IB_$r.root > $OUT/logs_full/table5_${t}_IB_$r.log 2>&1
  echo "    electron_selection exit code $?"
done
echo "=== $(date '+%F %T')  ALL DONE"
