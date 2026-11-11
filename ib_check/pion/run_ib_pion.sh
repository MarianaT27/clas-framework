#!/bin/bash
# =====================================================================================
# run_ib_pion.sh : IB/OB charged-pion cut check (common note Sec. 5), all DST files of each run.
#
# Run from the clas-framework folder (build with the pion_ib_check_v1 patch):
#   mkdir -p /volatile/clas12/mtenorio/pi0/ib_check/pion
#   nohup bash ib_check/pion/run_ib_pion.sh >& /volatile/clas12/mtenorio/pi0/ib_check/pion/run_ib_pion.log &
# Only one binary (v1.1): set BINS, e.g. in tcsh
#   env BINS=pion_analysis nohup bash ib_check/pion/run_ib_pion.sh >& /volatile/clas12/mtenorio/pi0/ib_check/pion/run_vz.log &
# All 6 target/polarity configurations at the same time (v1.4): set PARALLEL=1, e.g.
#   env BINS=pion_analysis PARALLEL=1 nohup bash ib_check/pion/run_ib_pion.sh >& /volatile/clas12/mtenorio/pi0/ib_check/pion/run_dvz.log &
#
# Runs (all on Lamiaa's run lists, all DST files on /cache):
#   IB : LD2 18335 18309 18310 | CxC 18344 18401 | CuSn 18384 18379
#   OB : LD2 18435             | CxC 18445       | CuSn 18591
# The runs of one target and polarity are read together (one output per target/polarity).
#
# For each target and polarity:
#   pion_analysis  -> root/pion_ana_<T>_<POL>.root   histograms for the figures (no pion cuts)
#   pion_selection -> root/pion_sel_<T>_<POL>.root   pi+/pi- cutflow (printed in logs/pion_sel_<T>_<POL>.log)
# The trigger electron uses the electron cuts of its polarity:
#   SF   : SamplingFractionService (IB = mini note tab:sf-params-ib-new, OB = common note Table 13)
#   DC   : IB 2.58 / 2.51 / 1.37 cm (mini note tab:dc-cuts-ib), OB 1.88 / 2.08 / 8.62 cm (common note Table 3)
#   PCAL : v, w > 9 cm (both)
# Pion cuts in pion_selection: Delta-t from Table 6 (OB) / Table 7 (IB); DC fiducial with the common-note
# values (Sec. 5.4) for both polarities; Delta-Vz switched off (no IB values yet).
# =====================================================================================
set -u
OUT=/volatile/clas12/mtenorio/pi0/ib_check/pion
DST=/cache/clas12/rg-d/production/pass1/recon
[ -x build/analyses/pion_analysis ] && [ -x build/analyses/pion_selection ] || { echo "run this from the clas-framework folder, after building"; exit 1; }
mkdir -p $OUT/root $OUT/logs $OUT/config

declare -A RUNS=(
  [LD2_IB]="18335 18309 18310" [CxC_IB]="18344 18401" [CuSn_IB]="18384 18379"
  [LD2_OB]="18435"             [CxC_OB]="18445"       [CuSn_OB]="18591"
)

write_config() {   # $1 binary  $2 target  $3 IB|OB   (input and output are given on the command line)
  local bin=$1 t=$2 pol=$3 polname r1 r2 r3
  if [ $pol = IB ]; then polname=Inbending;  r1=2.58; r2=2.51; r3=1.37
  else                   polname=Outbending; r1=1.88; r2=2.08; r3=8.62; fi
  local ele=ElectronAnalysis; [ $bin = pion_selection ] && ele=ElectronSelection
  cat <<TOML
[run]
num_threads = 8
percentage  = 100.0

[analysis]
target      = "$t"
polarity    = "$polname"
beam_energy = 10.54
version     = "2.0"

[$ele]
enable_ntuple            = false
enable_chi2pid_cut       = true
enable_momentum_cut      = true
enable_vz_cut            = true
enable_sf_cut            = true
enable_pcal_fiducial_cut = true
enable_dc_edge_cut       = true
pcal_lv_min              = 9.0
pcal_lw_min              = 9.0
dc_r1_edge_min           = $r1
dc_r2_edge_min           = $r2
dc_r3_edge_min           = $r3
TOML
  if [ $bin = pion_analysis ]; then cat <<TOML

[EventMonitor]

[PionAnalysis]
min_momentum                = 0.3
min_calo_energy             = 0.01
enable_forward_detector_cut = true
enable_momentum_cut         = true
enable_calorimeter_cut      = true
enable_dc_track_cut         = true
TOML
  else cat <<TOML

[PionSelection]
enable_ntuple              = false
enable_detector_region_cut = true
enable_fiducial_cuts       = true
enable_chi2pid_cut         = true
enable_momentum_cut        = true
enable_vz_cut              = true
enable_delta_vz_cut        = false
enable_delta_t_cut         = true
delta_t_sigma              = 3.0
TOML
  fi
}

run_key() {
  local key=$1 t pol files bin short cfg r
  t=${key%_*}; pol=${key#*_}
  : > $OUT/logs/files_${key}.txt
  for r in ${RUNS[$key]}; do ls -d $DST/$t/dst/recon/0$r/*.hipo >> $OUT/logs/files_${key}.txt; done
  files=$(paste -sd, $OUT/logs/files_${key}.txt)
  echo "=== $(date '+%F %T')  $key runs ${RUNS[$key]}: $(wc -l < $OUT/logs/files_${key}.txt) files"
  for bin in ${BINS:-pion_analysis pion_selection}; do
    short=${bin/pion_analysis/pion_ana}; short=${short/pion_selection/pion_sel}
    cfg=$OUT/config/${short}_${key}.toml
    write_config $bin $t $pol > $cfg
    ./build/analyses/$bin --config $cfg --input "$files" --output $OUT/root/${short}_${key}.root > $OUT/logs/${short}_${key}.log 2>&1
    echo "    $key $bin exit code $?"
  done
}

for key in LD2_IB CxC_IB CuSn_IB LD2_OB CxC_OB CuSn_OB; do
  if [ "${PARALLEL:-0}" = 1 ]; then run_key $key & else run_key $key; fi
done
wait
echo "=== $(date '+%F %T')  ALL DONE"
