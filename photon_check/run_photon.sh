#!/bin/bash
# =====================================================================================
# run_photon.sh : photon identification check, one SIDIS-train file per target and polarity.
#
# Run from the clas-framework folder, after building (photon-cut-check branch):
#   mkdir -p /volatile/clas12/mtenorio/pi0/photon_check
#   nohup bash photon_check/run_photon.sh >& /volatile/clas12/mtenorio/pi0/photon_check/run_photon.log &
#
# Runs: photon_check/runs.txt, one line per target and polarity, "<target> <OB|IB> <run>", e.g.
#   LD2 OB 18543
# Targets LD2, CxC, CuSn. Lines starting with # are skipped. Every line is run, one after the other.
#
# Input : /cache/clas12/rg-d/production/pass1/recon/<T>/dst/train/sidis/sidis_<run, 6 digits>.hipo
#         (SIDIS train: FD trigger electron, p > 1.8 GeV, vz < 5 cm, Q2 > 0.8 GeV2, W > 1.7 GeV,
#          1 in 5 events kept for 0.8 < Q2 < 2.0 GeV2; no cut on photons)
# Output: $OUT/root/photon_<T>_<POL>_<run>.root     histograms (PhotonAnalysis/...)
#         $OUT/config/photon_<T>_<POL>_<run>.toml   the config each run used
#         $OUT/runs.txt                             copy of the run list
# Log   : this script's output only (the nohup line above): one log for the whole run, with the
#         git commit, every binary's output and its cutflow (the loss table).
#
# Electron cuts of each polarity, as in the pion check (ib_check/pion/run_ib_pion.sh):
#   SF   : SamplingFractionService (IB = mini note tab:sf-params-ib-new, OB = common note Table 13)
#   DC   : IB 2.58 / 2.51 / 1.37 cm (mini note tab:dc-cuts-ib), OB 1.88 / 2.08 / 8.62 cm (common note Table 3)
#   PCAL : v, w > 9 cm (both)
# DIS: standard region of the pi0 analysis (1 <= Q2 <= 20 GeV2, 2 <= W <= 25 GeV, y <= 0.85).
# Photon cuts: the PhotonAnalysisAlg defaults = the production photon selection (photon_check/PROVENANCE.md).
# =====================================================================================
set -u
OUT=/volatile/clas12/mtenorio/pi0/photon_check
TRAIN=/cache/clas12/rg-d/production/pass1/recon
RUNLIST=photon_check/runs.txt
THREADS=${THREADS:-4}
[ -x build/analyses/photon_analysis ] || { echo "run this from the clas-framework folder, after building"; exit 1; }
[ -s $RUNLIST ] || { echo "$RUNLIST is missing or empty"; exit 1; }
mkdir -p $OUT/root $OUT/config
cp $RUNLIST $OUT/runs.txt

echo "=== $(date '+%F %T')  clas-framework $(git rev-parse --abbrev-ref HEAD) $(git rev-parse --short HEAD), $(git status --porcelain --untracked-files=no | wc -l) modified tracked files"

write_config() {   # $1 target  $2 IB|OB
  local t=$1 pol=$2 polname r1 r2 r3
  if [ $pol = IB ]; then polname=Inbending;  r1=2.58; r2=2.51; r3=1.37
  else                   polname=Outbending; r1=1.88; r2=2.08; r3=8.62; fi
  cat <<TOML
[run]
num_threads = $THREADS
percentage  = 100.0

[analysis]
target      = "$t"
polarity    = "$polname"
beam_energy = 10.54
version     = "2.0"

[ElectronSelection]
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

[PhotonAnalysis]
enable_energy_cut        = true
enable_pcal_energy_cut   = true
enable_theta_cut         = true
enable_gbt_cut           = true
enable_pcal_fiducial_cut = true
enable_beta_cut          = true
enable_e_gamma_angle_cut = true
min_energy               = 0.2
theta_min                = 5.0
theta_max                = 35.0
gbt_threshold            = 0.78
pcal_lv_min              = 14.0
pcal_lw_min              = 14.0
beta_min                 = 0.9
beta_max                 = 1.1
min_e_gamma_angle        = 8.0
TOML
}

while read -r t pol run rest; do
  case "$t" in ''|\#*) continue ;; esac
  case "$t" in LD2|CxC|CuSn) ;; *) echo "skipping line '$t $pol $run': target must be LD2, CxC or CuSn"; continue ;; esac
  case "$pol" in OB|IB) ;; *) echo "skipping line '$t $pol $run': polarity must be OB or IB"; continue ;; esac
  file=$(printf "%s/%s/dst/train/sidis/sidis_%06d.hipo" $TRAIN $t $run)
  [ -f $file ] || { echo "=== $t $pol run $run: $file not found, skipped"; continue; }
  key=${t}_${pol}_${run}
  write_config $t $pol > $OUT/config/photon_$key.toml
  echo "=== $(date '+%F %T')  $t $pol run $run: $file"
  ./build/analyses/photon_analysis --config $OUT/config/photon_$key.toml --input "$file" --output $OUT/root/photon_$key.root < /dev/null
  echo "=== $(date '+%F %T')  $t $pol run $run: photon_analysis exit code $?"
done < $RUNLIST
echo "=== $(date '+%F %T')  ALL DONE"
