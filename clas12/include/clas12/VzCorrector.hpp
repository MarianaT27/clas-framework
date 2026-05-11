/**
 * @file vz_corrector.h
 * @brief Header-only port of `apply_correction` from `vz_correction.py`,
 *        with per-peak n-sigma cuts and multi-variant support.
 *
 * @details
 * Single C++17 header. No ROOT / Boost dependency. One `VzCorrector`
 * instance owns the params for **every** `(target, polarity)` variant in
 * `vz_correction.VARIANTS`; the active variant is selected at runtime via
 * `set_variant()` and reused by `apply()` and the n-sigma cut helpers.
 *
 * @par Workflow
 * @parblock
 * 1. Run `vz_correction.py` (Python) to produce the per-variant npz files
 *    under `output/npz/`.
 * 2. Run `export_cpp_params.py --out output/cpp/vz_corrector_params.txt`
 *    once to bundle them into a single text file readable by this header.
 * 3. In your C++ analyzer, load the file and use the corrector:
 * @endparblock
 *
 * @par Algorithm (one-paragraph summary)
 * For each event, the corrector looks up cubic-in-theta polynomial
 * coefficients from the bin
 * `(sector, p_bin, phi_bin)` of the active variant, evaluates per-peak
 * `(mu_lo, sigma_lo)` and `(mu_hi, sigma_hi)` predictions, computes
 * z-scores against each peak, standardises to the reference
 * `(mu0_lo, sigma0_lo)` / `(mu0_hi, sigma0_hi)`, then sigmoid-blends the
 * two corrected vz hypotheses by the posterior `P(lo | vz)` under a
 * two-Gaussian mixture with equal priors. Mirrors `apply_correction()` in
 * `vz_correction.py` line for line.
 *
 * @par Example
 * @code
 *   #include "vz_corrector.h"
 *   auto corr = VzCorrector::load_from_file(
 *       "output/cpp/vz_corrector_params.txt");
 *   corr.set_variant("cusn", "outbending");          // or "cxc_inbending"
 *   double vz_c = corr.apply(vz, p, theta, phi_deg, sector);
 *   if (corr.pass_any(vz_c)) {
 *       // keep event
 *   }
 * @endcode
 *
 * @par RDataFrame
 * @code
 *   auto df2 = df
 *     .Define("vz_corr", [&](double vz, double p, double th,
 *                            double phi, int sec){
 *         return corr.apply(vz, p, th, phi, sec);
 *     }, {"vz","p","theta","phi_deg","sector"})
 *     .Filter([&](double vc){ return corr.pass_any(vc); }, {"vz_corr"});
 * @endcode
 *
 * @par Overriding the cut window
 * To use the latest fit-derived peak parameters from
 * `vz_fit_corrected.py` instead of the reference `(mu0, sigma0)` baked
 * into the params file, call `corr.set_cut(mu_lo, sigma_lo, mu_hi,
 * sigma_hi)` after `set_variant()`. The override is per-variant: changing
 * the active variant restores its own reference parameters (unless
 * `set_cut` is called again).
 *
 * @par Thread safety
 * Not thread-safe. The active-variant pointer and the cut parameters are
 * mutable per-instance state. Use one `VzCorrector` per thread or
 * synchronise externally.
 *
 * @see apply_correction in vz_correction.py
 * @see export_cpp_params.py for the params-file producer
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * @namespace vz_corrector_detail
 * @brief Implementation detail; not intended for direct use.
 */
namespace vz_corrector_detail {

/**
 * @brief Three-argument clamp returning `v` constrained to `[lo, hi]`.
 *
 * @details Provided so this header builds against pre-C++17 toolchains
 * that lack `std::clamp`. Uses only `operator<` on `T` and assumes
 * `lo <= hi`.
 *
 * @tparam T Any totally-ordered comparable type.
 * @param  v  The value to clamp.
 * @param  lo Lower bound (inclusive).
 * @param  hi Upper bound (inclusive).
 * @return `lo` if `v < lo`; `hi` if `hi < v`; otherwise `v`.
 */
template <typename T>
inline T clamp_(T v, T lo, T hi) { return v < lo ? lo : (hi < v ? hi : v); }

}  // namespace vz_corrector_detail
using vz_corrector_detail::clamp_;


/**
 * @class VzCorrector
 * @brief Multi-variant vertex-z correction + per-peak n-sigma cut.
 *
 * @details
 * Holds the polynomial parameter sets for every `(target, polarity)`
 * variant from `vz_correction.VARIANTS`. After construction (typically
 * via `load_from_file()`) the caller must select an active variant using
 * `set_variant()` before invoking `apply()`, `pass_lo()`, `pass_hi()`,
 * `pass_any()`, `which_peak()`, or `set_cut()`. The exception is when
 * the parameters file contains exactly one variant — in that case it is
 * auto-selected at load time.
 *
 * @invariant `n_sigma_ >= 0` (no enforcement; pathological values just
 *            reject every event).
 * @invariant After a successful `set_variant()`, `active_` points into
 *            `variants_` and remains valid for the corrector's lifetime
 *            (no rehashing happens after `load_from_file`).
 */
class VzCorrector {
 public:
  /// CLAS12 sector phi centers (degrees), indexed by `sector - 1`.
  /// Matches `SECTOR_PHI_CENTER` in `vz_correction.py`.
  static constexpr std::array<double, 6> kSectorPhiCenter = {
      0.0, 60.0, 120.0, 180.0, -120.0, -60.0,
  };
  static constexpr int    kNSectors        = 6;     ///< Number of CLAS12 sectors.
  static constexpr int    kNPhi            = 6;     ///< Per-sector phi-bin count.
  static constexpr int    kNPoly           = 4;     ///< Polynomial order + 1 (cubic).
  static constexpr double kPhiHalfWidth    = 30.0;  ///< Half-range of local phi (deg).
  static constexpr double kPhiBinWidth     = 2.0 * kPhiHalfWidth / kNPhi; ///< Local phi-bin width (deg).
  static constexpr double kThetaLoFallback = 6.0;   ///< Fallback `tmin` for cells with NaN domain (matches `TH_LO`).
  static constexpr double kThetaHiFallback = 30.0;  ///< Fallback `tmax` for cells with NaN domain (matches `TH_HI`).
  static constexpr double kSigmaFloor      = 0.01;  ///< Lower clamp on per-cell predicted sigmas (matches `sig_*_safe`).

  /**
   * @struct Params
   * @brief Per-variant parameter set.
   *
   * @details Contains the global reference (post-correction) peak
   * parameters, the p-binning recovered from the npz, the per-cell
   * cubic coefficients (one cell per `(sector, p-bin, phi-bin)` triplet)
   * and the per-cell theta clip ranges. All vector sizes are
   * `kNSectors * n_p * kNPhi`; flat indexing is via `cell_index()`.
   */
  struct Params {
    double mu0_lo    = 0.0;  ///< Reference lo-peak center post-correction (cm).
    double sigma0_lo = 1.0;  ///< Reference lo-peak width post-correction (cm).
    double mu0_hi    = 0.0;  ///< Reference hi-peak center post-correction (cm).
    double sigma0_hi = 1.0;  ///< Reference hi-peak width post-correction (cm).
    double p_lo      = 0.0;  ///< Lower edge of the first p-bin (GeV/c).
    double p_binw    = 1.0;  ///< Width of each p-bin (GeV/c).
    int    n_p       = 0;    ///< Number of p-bins.

    /// Lo-peak `mu(theta)` cubic coefficients (lowest order first), per cell.
    std::vector<std::array<double, kNPoly>> mu_lo;
    /// Hi-peak `mu(theta)` cubic coefficients (lowest order first), per cell.
    std::vector<std::array<double, kNPoly>> mu_hi;
    /// Lo-peak `sigma(theta)` cubic coefficients (lowest order first), per cell.
    std::vector<std::array<double, kNPoly>> sig_lo;
    /// Hi-peak `sigma(theta)` cubic coefficients (lowest order first), per cell.
    std::vector<std::array<double, kNPoly>> sig_hi;
    /// Per-cell `[theta_min, theta_max]` clip range (degrees). NaN entries
    /// fall back to `[kThetaLoFallback, kThetaHiFallback]`.
    std::vector<std::array<double, 2>> theta_dom;

    /**
     * @brief Flat index into the per-cell vectors.
     *
     * @details Layout is `(sector - 1) * n_p * kNPhi + ip * kNPhi + iphi`,
     *          which matches the iteration order produced by
     *          `export_cpp_params.py`.
     *
     * @param sector CLAS12 sector index in `[1, kNSectors]`.
     * @param ip     p-bin index in `[0, n_p)`.
     * @param iphi   phi-bin index in `[0, kNPhi)`.
     * @return       Flat index in `[0, kNSectors * n_p * kNPhi)`.
     */
    std::size_t cell_index(int sector, int ip, int iphi) const {
      return static_cast<std::size_t>((sector - 1) * n_p * kNPhi
                                      + ip * kNPhi + iphi);
    }
  };

  /**
   * @brief Construct an empty corrector.
   *
   * @details The instance has no variants and no active selection. To be
   *          useful, populate it via the static `load_from_file()` factory.
   *
   * @param n_sigma_cut Half-width of the per-peak window in units of the
   *                    reference sigma. Default `3.0` matches the typical
   *                    CLAS12 vertex selection.
   */
  explicit VzCorrector(double n_sigma_cut = 3.0) : n_sigma_(n_sigma_cut) {}

  /**
   * @brief Load every variant from a multi-variant params file.
   *
   * @details The file format is documented above the out-of-class
   *          definition of this function. `export_cpp_params.py` is the
   *          canonical producer.
   *
   *          When the file contains exactly one variant, that variant is
   *          auto-selected (no `set_variant()` call required).
   *
   * @param path        Path to the params text file.
   * @param n_sigma_cut Initial cut half-width (see ctor).
   * @return            A populated `VzCorrector`.
   *
   * @throws std::runtime_error if the file cannot be opened, the global
   *         header is missing or malformed, the schema (`n_phi`,
   *         `n_sectors`, `n_poly`) does not match the compile-time
   *         constants, a variant block is truncated, or a per-cell row
   *         is malformed.
   */
  static VzCorrector load_from_file(const std::string& path,
                                    double n_sigma_cut = 3.0);

  /**
   * @brief Select the active variant by full key.
   *
   * @details The key is the `Variant.name` field from
   *          `vz_correction.VARIANTS`, e.g. `"cusn_outbending"`,
   *          `"cxc_inbending"`. After a successful call, `apply()` and
   *          the cut helpers operate on this variant's parameters.
   *
   * @param key Variant identifier as written in the params file.
   * @throws std::runtime_error if `key` is not present in the loaded file.
   */
  void set_variant(const std::string& key) {
    auto it = variants_.find(key);
    if (it == variants_.end())
      throw std::runtime_error("VzCorrector: unknown variant '" + key + "'");
    active_key_ = key;
    active_ = &it->second;
  }

  /**
   * @brief Convenience overload: select by `(target, polarity)`.
   *
   * @details Joins the two parts with an underscore and forwards to the
   *          single-string overload. So `set_variant("cusn", "outbending")`
   *          is equivalent to `set_variant("cusn_outbending")`.
   *
   * @param target   Target tag (e.g. `"cusn"`, `"cxc"`).
   * @param polarity Polarity tag (e.g. `"outbending"`, `"inbending"`).
   * @throws std::runtime_error if the combined key is unknown.
   */
  void set_variant(const std::string& target, const std::string& polarity) {
    set_variant(target + "_" + polarity);
  }

  /**
   * @brief List every variant key currently loaded.
   *
   * @return A vector of the keys present in the params file.
   *
   * @note Order is unspecified (backed by `std::unordered_map`). Sort the
   *       result if you need deterministic output.
   */
  std::vector<std::string> variants() const {
    std::vector<std::string> keys;
    keys.reserve(variants_.size());
    for (const auto& kv : variants_) keys.push_back(kv.first);
    return keys;
  }

  /**
   * @brief Key of the currently selected variant.
   *
   * @return The selected key, or an empty string if none has been set
   *         and the file held more than one variant.
   */
  const std::string& active_variant() const { return active_key_; }

  /**
   * @brief Map `(phi, sector)` to a phi-bin index.
   *
   * @details Mirrors `phi_bin_index()` in `vz_correction.py`. Wraps phi
   *          to the local sector frame in `[-180, 180]`, clips into
   *          `[-kPhiHalfWidth, kPhiHalfWidth)`, then bins.
   *
   * @param phi_deg Track phi in **degrees** in the global lab frame
   *                (the same convention as the Python pipeline).
   * @param sector  CLAS12 sector in `[1, kNSectors]`. Out-of-range values
   *                are clamped to the nearest sector.
   * @return        phi-bin index in `[0, kNPhi)`.
   *
   * @see phi_bin_index in vz_correction.py
   */
  static int phi_bin_index(double phi_deg, int sector) {
    const int s = clamp_(sector, 1, kNSectors);
    const double center = kSectorPhiCenter[s - 1];
    double r = std::fmod(phi_deg - center + 540.0, 360.0);
    if (r < 0.0) r += 360.0;                          // match Python's %
    double phi_local = r - 180.0;
    if (phi_local < -kPhiHalfWidth)            phi_local = -kPhiHalfWidth;
    else if (phi_local > kPhiHalfWidth - 1e-9) phi_local = kPhiHalfWidth - 1e-9;
    int iphi = static_cast<int>((phi_local + kPhiHalfWidth) / kPhiBinWidth);
    return clamp_(iphi, 0, kNPhi - 1);
  }

  /**
   * @brief Apply the two-target sigmoid-blended vz correction to one event.
   *
   * @details Looks up per-cell cubic coefficients in the active variant,
   *          evaluates `(mu, sigma)` predictions for both peaks at the
   *          (clipped) theta, computes per-peak z-scores, standardises
   *          to the reference `(mu0_lo, sigma0_lo)` / `(mu0_hi,
   *          sigma0_hi)`, and blends the two hypotheses with the
   *          sigmoid posterior under a two-Gaussian mixture.
   *
   *          Out-of-range `p` and `theta` are clamped (to the binning
   *          range and the per-cell domain respectively); out-of-range
   *          `sector` is clamped to `[1, kNSectors]`.
   *
   * @param vz       Raw vertex-z (cm).
   * @param p        Track momentum (GeV/c).
   * @param theta    Track polar angle (degrees).
   * @param phi_deg  Track phi (degrees, lab frame).
   * @param sector   CLAS12 sector in `[1, kNSectors]`.
   * @return         Corrected vz (cm), standardised so that lo-peak
   *                 events sit at `mu0_lo` with width `sigma0_lo` and
   *                 hi-peak events at `mu0_hi` with width `sigma0_hi`.
   *
   * @throws std::runtime_error if no variant has been selected.
   *
   * @see apply_correction in vz_correction.py for the reference
   *      Python implementation.
   */
  double apply(double vz, double p, double theta,
               double phi_deg, int sector) const {
    const Params& pr = active();
    const int s_clamped = clamp_(sector, 1, kNSectors);
    const int ip = clamp_(
        static_cast<int>((p - pr.p_lo) / pr.p_binw), 0, pr.n_p - 1);
    const int iphi = phi_bin_index(phi_deg, sector);
    const std::size_t idx = pr.cell_index(s_clamped, ip, iphi);

    double tmin = pr.theta_dom[idx][0];
    double tmax = pr.theta_dom[idx][1];
    if (!std::isfinite(tmin)) tmin = kThetaLoFallback;
    if (!std::isfinite(tmax)) tmax = kThetaHiFallback;
    const double th_c = clamp_(theta, tmin, tmax);

    // Local Horner evaluator, lowest-order coefficient first.
    auto poly = [](const std::array<double, kNPoly>& c, double x) {
      double y = c[kNPoly - 1];
      for (int k = kNPoly - 2; k >= 0; --k) y = y * x + c[k];
      return y;
    };

    const double mu_lo  = poly(pr.mu_lo[idx],  th_c);
    const double mu_hi  = poly(pr.mu_hi[idx],  th_c);
    const double sig_lo = std::max(poly(pr.sig_lo[idx], th_c), kSigmaFloor);
    const double sig_hi = std::max(poly(pr.sig_hi[idx], th_c), kSigmaFloor);

    const double z_lo = (vz - mu_lo) / sig_lo;
    const double z_hi = (vz - mu_hi) / sig_hi;
    const double vz_lo = pr.mu0_lo + pr.sigma0_lo * z_lo;
    const double vz_hi = pr.mu0_hi + pr.sigma0_hi * z_hi;

    const double log_ratio =
        0.5 * (z_hi * z_hi - z_lo * z_lo) + std::log(sig_hi / sig_lo);
    const double w_lo = 1.0 / (1.0 + std::exp(-log_ratio));
    return vz_hi + w_lo * (vz_lo - vz_hi);
  }

  /**
   * @brief n-sigma test against the lo peak window on **corrected** vz.
   *
   * @details Returns `|vz_corr - mu0_lo| < n_sigma * sigma0_lo` using
   *          the active variant's reference parameters (or whatever
   *          `set_cut()` last installed).
   *
   * @param vz_corr Corrected vz from `apply()` (cm).
   * @return        `true` if the event is within the lo-peak window.
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  bool pass_lo(double vz_corr) const {
    const Params& p = active();
    return std::fabs(vz_corr - p.mu0_lo) < n_sigma_ * p.sigma0_lo;
  }

  /**
   * @brief n-sigma test against the hi peak window on **corrected** vz.
   *
   * @details Returns `|vz_corr - mu0_hi| < n_sigma * sigma0_hi` using
   *          the active variant's reference parameters (or whatever
   *          `set_cut()` last installed).
   *
   * @param vz_corr Corrected vz from `apply()` (cm).
   * @return        `true` if the event is within the hi-peak window.
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  bool pass_hi(double vz_corr) const {
    const Params& p = active();
    return std::fabs(vz_corr - p.mu0_hi) < n_sigma_ * p.sigma0_hi;
  }

  /**
   * @brief n-sigma test against either peak window.
   *
   * @param vz_corr Corrected vz from `apply()` (cm).
   * @return        `pass_lo(vz_corr) || pass_hi(vz_corr)`.
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  bool pass_any(double vz_corr) const {
    return pass_lo(vz_corr) || pass_hi(vz_corr);
  }

  /**
   * @brief Identify which peak (if any) accepts the event.
   *
   * @details If both windows accept (rare overlap when the cuts are
   *          loose), the peak with the smaller `|z|` distance to the
   *          event wins.
   *
   * @param vz_corr Corrected vz from `apply()` (cm).
   * @return  `-1` if neither window accepts;
   *          `0`  if the lo peak (or the closer one in case of overlap);
   *          `1`  if the hi peak.
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  int which_peak(double vz_corr) const {
    const bool lo = pass_lo(vz_corr);
    const bool hi = pass_hi(vz_corr);
    if (lo && !hi) return 0;
    if (hi && !lo) return 1;
    if (!lo && !hi) return -1;
    const Params& p = active();
    const double zl = std::fabs(vz_corr - p.mu0_lo) / p.sigma0_lo;
    const double zh = std::fabs(vz_corr - p.mu0_hi) / p.sigma0_hi;
    return zl <= zh ? 0 : 1;
  }

  /**
   * @brief Override the active variant's per-peak cut center / width.
   *
   * @details Useful to swap the loaded reference `(mu0, sigma0)` for
   *          freshly fitted values from `vz_fit_corrected.py`. Only the
   *          cut window is affected; the correction itself
   *          (per-cell polynomials, p-binning, theta clip range) is not
   *          touched. Other variants are unaffected.
   *
   * @param mu_lo    New lo-peak center (cm).
   * @param sigma_lo New lo-peak width  (cm).
   * @param mu_hi    New hi-peak center (cm).
   * @param sigma_hi New hi-peak width  (cm).
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  void set_cut(double mu_lo, double sigma_lo,
               double mu_hi, double sigma_hi) {
    Params& p = mutable_active();
    p.mu0_lo = mu_lo;  p.sigma0_lo = sigma_lo;
    p.mu0_hi = mu_hi;  p.sigma0_hi = sigma_hi;
  }

  /**
   * @brief Set the n-sigma half-width for `pass_*` and `which_peak`.
   *
   * @details Applies to the corrector instance as a whole — affects
   *          every variant, current and future. Default at construction
   *          time is `3.0`.
   *
   * @param n New half-width in units of the reference sigma.
   *          Pathological values (`< 0` or NaN) reject every event but
   *          do not raise.
   */
  void set_n_sigma(double n) { n_sigma_ = n; }

  /**
   * @brief Access the active variant's parameters (read-only).
   *
   * @return Const reference to the parameter set selected by
   *         `set_variant()`.
   *
   * @throws std::runtime_error if no variant has been selected.
   */
  const Params& active() const {
    if (!active_)
      throw std::runtime_error(
          "VzCorrector: no active variant. Call set_variant() first.");
    return *active_;
  }

  /**
   * @brief Current n-sigma cut half-width.
   * @return The value last set via the constructor or `set_n_sigma()`.
   */
  double n_sigma() const { return n_sigma_; }

 private:
  /**
   * @internal
   * @brief Mutable counterpart of `active()`, used by `set_cut()`.
   *
   * @return Mutable reference to the active variant's parameters.
   * @throws std::runtime_error if no variant has been selected.
   */
  Params& mutable_active() {
    if (!active_)
      throw std::runtime_error(
          "VzCorrector: no active variant. Call set_variant() first.");
    return *active_;
  }

  std::unordered_map<std::string, Params> variants_;  ///< All loaded variants, keyed by name.
  Params* active_ = nullptr;                          ///< Points into `variants_`; null until `set_variant()`.
  std::string active_key_;                            ///< Key matching `*active_` (empty when unset).
  double n_sigma_;                                    ///< Cut half-width in units of the reference sigma.
};


/**
 * @brief Out-of-class definition of `VzCorrector::load_from_file`.
 *
 * @par Wire format (text, whitespace-separated, `#`-prefixed lines ignored)
 * @parblock
 * @code
 *   # Global header (one line):
 *   n_variants n_phi n_sectors n_poly
 *
 *   # Then one block per variant:
 *   variant_key mu0_lo sigma0_lo mu0_hi sigma0_hi p_lo p_binw n_p
 *   sector ip iphi  c_mu_lo[4]  c_mu_hi[4]  c_sig_lo[4]  c_sig_hi[4]  th_min th_max
 *   ...                                                       (n_sectors * n_p * n_phi rows)
 * @endcode
 * @endparblock
 *
 * The schema fields `n_phi`, `n_sectors`, `n_poly` must match
 * `kNPhi`, `kNSectors`, `kNPoly` respectively, otherwise the loader
 * throws. Polynomial coefficients are written lowest-order first.
 *
 * Producer of this format: `export_cpp_params.py`.
 *
 * @see VzCorrector::load_from_file
 */
inline VzCorrector VzCorrector::load_from_file(const std::string& path,
                                               double n_sigma_cut) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("VzCorrector: cannot open " + path);

  // Pull the next non-blank, non-comment line; returns false on EOF.
  auto next_data = [&](std::string& out) -> bool {
    while (std::getline(in, out)) {
      const auto start = out.find_first_not_of(" \t\r\n");
      if (start == std::string::npos) continue;
      if (out[start] == '#') continue;
      out = out.substr(start);
      return true;
    }
    return false;
  };

  VzCorrector vc(n_sigma_cut);
  std::string line;

  // ── Global header: n_variants n_phi n_sectors n_poly ────────────────
  if (!next_data(line))
    throw std::runtime_error("VzCorrector: missing global header in " + path);
  int n_variants = 0, n_phi_chk = 0, n_sec_chk = 0, n_poly_chk = 0;
  {
    std::istringstream iss(line);
    iss >> n_variants >> n_phi_chk >> n_sec_chk >> n_poly_chk;
    if (!iss)
      throw std::runtime_error("VzCorrector: bad global header in " + path);
    if (n_phi_chk != kNPhi || n_sec_chk != kNSectors || n_poly_chk != kNPoly)
      throw std::runtime_error("VzCorrector: schema mismatch in " + path);
  }

  // ── Per-variant blocks ──────────────────────────────────────────────
  for (int v = 0; v < n_variants; ++v) {
    if (!next_data(line))
      throw std::runtime_error(
          "VzCorrector: truncated at variant " + std::to_string(v));
    std::string key;
    Params p;
    {
      std::istringstream iss(line);
      iss >> key >> p.mu0_lo >> p.sigma0_lo >> p.mu0_hi >> p.sigma0_hi
          >> p.p_lo >> p.p_binw >> p.n_p;
      if (!iss)
        throw std::runtime_error(
            "VzCorrector: bad variant header for '" + key + "'");
    }

    const std::size_t n_cells =
        static_cast<std::size_t>(kNSectors) * p.n_p * kNPhi;
    p.mu_lo.assign(n_cells, {});
    p.mu_hi.assign(n_cells, {});
    p.sig_lo.assign(n_cells, {});
    p.sig_hi.assign(n_cells, {});
    p.theta_dom.assign(n_cells, {0.0, 0.0});

    for (std::size_t i = 0; i < n_cells; ++i) {
      if (!next_data(line))
        throw std::runtime_error(
            "VzCorrector: truncated cells in variant '" + key + "'");
      std::istringstream iss(line);
      int sec = 0, ip = 0, iphi = 0;
      iss >> sec >> ip >> iphi;
      const auto idx = p.cell_index(sec, ip, iphi);
      auto read4 = [&](std::array<double, kNPoly>& dst) {
        for (int k = 0; k < kNPoly; ++k) iss >> dst[k];
      };
      read4(p.mu_lo[idx]);
      read4(p.mu_hi[idx]);
      read4(p.sig_lo[idx]);
      read4(p.sig_hi[idx]);
      iss >> p.theta_dom[idx][0] >> p.theta_dom[idx][1];
      if (!iss)
        throw std::runtime_error(
            "VzCorrector: bad row in variant '" + key + "'");
    }
    vc.variants_.emplace(std::move(key), std::move(p));
  }

  // Single-variant convenience: auto-select so callers don't have to.
  if (vc.variants_.size() == 1) {
    auto& only = *vc.variants_.begin();
    vc.active_ = &only.second;
    vc.active_key_ = only.first;
  }
  return vc;
}
