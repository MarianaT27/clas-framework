# Photon identification check: provenance

Where every number used by `photon_check/` comes from. File and line references are to the
`photon-cut-check` branch of clas-framework, which starts from `electron-cut-check` at commit 64e7655.
The commit each set of results was made with is in the first line of `run_photon.log`.

## Data

- **Input:** RG-D pass-1 SIDIS train, one file per run:
  `/cache/clas12/rg-d/production/pass1/recon/<T>/dst/train/sidis/sidis_<run>.hipo`.
- **Runs:** `photon_check/runs.txt`, one OB and one IB run per target (LD2, CxC, CuSn). The list is
  copied to the output folder with the results.
- **Train selection (RGDSidisWagon), before anything in this check:** at least one Forward Detector
  trigger electron (pid 11, -4000 < status <= -2000) with p > 1.8 GeV, vz < 5 cm, Q² > 0.8 GeV² and
  W > 1.7 GeV, computed with a 10.54 GeV beam on a proton target (the same cuts as the CT train).
  Events with Q² > 2.0 GeV² are all kept; events with 0.8 < Q² < 2.0 GeV² are prescaled, 1 in 5 kept
  (by event number). **The train applies no cut on photons**, so the photon losses start from every
  pid-22 particle in the train events.

## Events: electron and DIS selection (before the photons)

The same chain as the pi0 analysis (`analyses/pi0_multiplicity/pi0_multiplicity.cpp`).

- **Electron:** `ElectronSelectionAlg` with every electron cut on, with the values of the pion check
  (`ib_check/pion/run_ib_pion.sh`):
  - SF: `SamplingFractionService` (IB from the mini note, `tab:sf-params-ib-new`; OB from common note Table 13).
  - DC edges: IB 2.58 / 2.51 / 1.37 cm (mini note `tab:dc-cuts-ib`); OB 1.88 / 2.08 / 8.62 cm (common note Table 3).
  - PCAL fiducial: v, w > 9 cm (both polarities).
  - Plus the chi2pid, momentum and vz cuts of `ElectronSelectionAlg`.
- **DIS:** `DISAnalysis::standard_dis_region()` (`clas12/include/clas12/physics/DISAnalysis.hpp:82`):
  1 <= Q² <= 20 GeV², 2 <= W <= 25 GeV, y <= 0.85.
- **Electron for theta(e, gamma):** the trigger electron selected above.

## Photon cuts (PhotonAnalysisAlg, in this order)

| Step | Cut | Value | Source in clas-framework |
|---|---|---|---|
| 1 | pid | 22 | `PhotonCutsService.hpp:124` (`select_photons_with_AI`) |
| 2 | E_gamma (= \|p\|) | >= 0.2 GeV | `PhotonCutsService.hpp:54` `MIN_PHOTON_ENERGY`, used in `pass_pid_purity_filter` (line 277) |
| 3 | E_PCAL | > 0 | `pass_pid_purity_filter` (line 277) |
| 4 | theta | 5 <= theta <= 35 deg | `PhotonCutsService.hpp:55-56` `MIN_FD_THETA`, `MAX_FD_THETA` |
| 5 | GBT score | > 0.78 | `PhotonCutsService.hpp:51` `AI_THRESHOLD` |
| 6 | PCAL fiducial | lv > 14 cm and lw > 14 cm | `PhotonCutsService.hpp:52-53`, `pass_pcal_fiducial_cut` (line 90) |
| 7 | beta | 0.9 < beta < 1.1 | `PhotonSelectionAlg.hpp:59-60` `min_beta`, `max_beta` |
| 8 | theta(e, gamma) | > 8 deg | `Pi0AnalysisAlg.hpp:671` `MIN_ELECTRON_PHOTON_ANGLE` |

- **Bounds:** steps 2 and 4 are inclusive; steps 3, 5, 6, 7 and 8 are strict, as in the code they
  come from.
- **Where the production selection applies them:** steps 1 to 6 are in `PhotonCutsService`
  (`select_photons_with_AI`) and `PhotonSelectionAlg`, step 7 in `PhotonSelectionAlg`, and step 8 in
  `Pi0AnalysisAlg` (on each photon, against the electron).
- **The defaults** of `PhotonAnalysisConfig` are these values. `run_photon.sh` also writes them
  explicitly into each config, which is saved with the results.

### GBT classifier (step 5)

- **Model:** CatBoost gradient-boosted trees, selected by run range and cooking pass
  (`PhotonCutsService.hpp`, `s_model_map`, lines 198-215; pass hard-coded to 1, line 180).
- **RG-D runs (18305 to 19131) match no entry:** the table covers RG-A (5032-5666, 6156-6783,
  11093-11571) and RG-C (16042-16772). RG-D therefore falls through to the default,
  `ApplyCatboostModel_RGA_inbending_pass1` (line 296), for **both OB and IB** runs. No warning is printed.
- **Input:** 45 features per photon (`score_photon`):
  - the photon: E, E_PCAL, theta, PCAL m2u, m2v;
  - the nearest electron: angular distance R and energy difference;
  - the 3 nearest photons, 2 nearest charged hadrons and 2 nearest neutral hadrons, each with
    R, dE, E_PCAL, m2u, m2v;
  - the number of photons within R < 0.1, 0.2 and 0.35.
- **Score:** sigmoid(model output) = 1 / (1 + exp(-x)), in [0, 1]. The check histograms it with
  `PhotonCutsService::gbt_scores`, the same code the production selection uses.

## Code added for this check

- **`PhotonCutsService::gbt_scores`:** returns the score of every pid-22 row that passes the
  pre-filter. `filter_photon` now calls the new `score_photon` and keeps a photon when its score is
  > `m_ai_threshold`. The arithmetic is unchanged, so the production selection is unchanged.
- **`PhotonAnalysisAlg`** (`algorithms/include/clas12/algorithms/analysis/PhotonAnalysisAlg.hpp`)
  and the **`photon_analysis`** executable (`analyses/photon/photon_analysis.cpp`).
- **`photon_check/run_photon.sh`** (runs) and **`photon_check/photon_figs.py`** (figures and loss table).

## What each figure shows

Each distribution is the photons that **reach** that cut, after all the previous steps, as raw counts
(not normalized).

- **Pre-filter figure:** all pid-22 photons, and the photons left after the three pre-filter cuts.
- **Loss table:** for each step, the photons that pass it, as a fraction of the previous step and
  of pid 22.
