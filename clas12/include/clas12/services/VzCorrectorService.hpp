#pragma once

/**
 * @file VzCorrectorService.hpp
 * @brief Service wrapping `VzCorrector` plus the Voigt-derived per-peak cut
 *        windows used to replace the legacy hard-coded V_z ranges.
 *
 * For solid targets (CuSn / CxC, in + out bending) the service:
 *   1. Loads the multi-variant correction parameters from the text file
 *      produced by `export_cpp_params.py`.
 *   2. Selects the active variant from `(target, polarity)`.
 *   3. Overrides the variant's stored reference (mu0, sigma0) with the
 *      Voigt-fit Gaussian-equivalent (mu, sigma_eff) per peak, so that
 *      `pass_lo/pass_hi/pass_any` test the +/- n_sigma_eff window.
 *
 * For LD2 the service stays disabled and `pass_vz_cut` falls back to the
 * legacy fixed range `[-15, 5] cm` on the raw vz.
 *
 * Pions (v1.3): when the params file also holds the pion blocks
 * `<variant>_pim` / `<variant>_pip` (produced by ib_check/pion/export_vz_params.py
 * from the pion-mode npz of vz_correction.py), one extra corrector per pion
 * charge is loaded, with the pion windows of common note Table 8
 * (`corrected_vz_pion`, `pass_vz_cut_pion`). Without those blocks the pion
 * functions fall back to the electron correction and windows (warning at init).
 *
 * Thread safety: `set_variant` and `set_cut` are called once during
 * `initialize()`. After init we only call `apply` / `pass_any` /
 * `pass_lo` / `pass_hi`, which are const and read from the stable
 * `active_` pointer. Multiple worker threads can therefore call into the
 * service concurrently without external synchronisation.
 */

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <clas12/Target.hpp>
#include <clas12/VzCorrector.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace framework {

class VzCorrectorService : public Service {
   public:
    static constexpr std::string_view service_name() { return "VzCorrectorService"; }

    // Voigt-fit Gaussian-equivalent (mu, sigma_eff) per peak, derived offline
    // from vz_fit_corrected.py and quoted in the analysis note (Table:Vz-cuts).
    struct VoigtWindow {
        double mu_lo;
        double sigma_lo;
        double mu_hi;
        double sigma_hi;
    };

    struct Config {
        std::string params_file = "data/Vz/vz_corrector_params.txt";
        Target      target      = Target::LD2;
        Polarity    polarity    = Polarity::Outbending;
        double      n_sigma     = 3.0;
        // Legacy fallback window applied to raw vz for LD2 and whenever the
        // correction cannot be loaded.
        double      ld2_vz_min  = -15.0;
        double      ld2_vz_max  =   5.0;
    };

    explicit VzCorrectorService(std::string name = "VzCorrectorService")
        : Service(std::move(name)) {}

    explicit VzCorrectorService(const Config& config, std::string name = "VzCorrectorService")
        : Service(std::move(name)), m_config(config) {}

    auto initialize() -> StatusCode override {
        if (m_config.target == Target::LD2) {
            Log::info(name(),
                      "Target is LD2 -> Vz correction disabled; using cut [{:.2f}, {:.2f}] cm",
                      m_config.ld2_vz_min, m_config.ld2_vz_max);
            m_loaded = false;
            return StatusCode::SUCCESS();
        }

        auto key = variant_key(m_config.target, m_config.polarity);
        if (!key) {
            Log::warning(name(),
                         "No Voigt-correction variant for target={} polarity={} -> Vz correction disabled",
                         to_string(m_config.target), to_string(m_config.polarity));
            m_loaded = false;
            return StatusCode::SUCCESS();
        }

        try {
            m_corrector = std::make_unique<VzCorrector>(
                VzCorrector::load_from_file(m_config.params_file, m_config.n_sigma));
            m_corrector->set_variant(*key);
            const VoigtWindow w = voigt_window(m_config.target, m_config.polarity);
            m_corrector->set_cut(w.mu_lo, w.sigma_lo, w.mu_hi, w.sigma_hi);
            m_loaded = true;
            m_active_key = *key;
            Log::info(name(),
                      "Loaded VzCorrector from '{}' variant '{}' (target={}, polarity={})",
                      m_config.params_file, *key,
                      to_string(m_config.target), to_string(m_config.polarity));
            Log::info(name(),
                      "Voigt cut: lo=[{:.3f} +/- {} x {:.3f}], hi=[{:.3f} +/- {} x {:.3f}]",
                      w.mu_lo, m_config.n_sigma, w.sigma_lo,
                      w.mu_hi, m_config.n_sigma, w.sigma_hi);
        } catch (const std::exception& e) {
            Log::error(name(), "Failed to load VzCorrector: {}", e.what());
            return StatusCode::FAILURE();
        }

        // Pion correctors (one per charge), same file, keys <variant>_pim / <variant>_pip.
        const auto keys = m_corrector->variants();
        for (int i = 0; i < 2; ++i) {
            const bool is_piplus = (i == 1);
            const std::string pkey = *key + (is_piplus ? "_pip" : "_pim");
            if (std::find(keys.begin(), keys.end(), pkey) == keys.end()) {
                Log::warning(name(),
                             "No pion variant '{}' in '{}' -> pi{} use the electron correction and windows",
                             pkey, m_config.params_file, is_piplus ? "+" : "-");
                continue;
            }
            try {
                m_pion[i] = std::make_unique<VzCorrector>(
                    VzCorrector::load_from_file(m_config.params_file, m_config.n_sigma));
                m_pion[i]->set_variant(pkey);
                const VoigtWindow w = pion_voigt_window(m_config.target, m_config.polarity, is_piplus);
                m_pion[i]->set_cut(w.mu_lo, w.sigma_lo, w.mu_hi, w.sigma_hi);
                Log::info(name(),
                          "Loaded pion variant '{}': Table 8 cut lo=[{:.3f} +/- {} x {:.3f}], hi=[{:.3f} +/- {} x {:.3f}]",
                          pkey, w.mu_lo, m_config.n_sigma, w.sigma_lo,
                          w.mu_hi, m_config.n_sigma, w.sigma_hi);
            } catch (const std::exception& e) {
                Log::error(name(), "Failed to load pion variant '{}': {}", pkey, e.what());
                return StatusCode::FAILURE();
            }
        }

        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override { return StatusCode::SUCCESS(); }

    void set_config(const Config& cfg) { m_config = cfg; }
    [[nodiscard]] auto config()        const -> const Config& { return m_config; }
    [[nodiscard]] auto is_loaded()     const -> bool          { return m_loaded; }
    [[nodiscard]] auto active_variant() const -> const std::string& { return m_active_key; }

    /// Apply the per-event correction (returns raw vz when no variant is loaded).
    [[nodiscard]] auto corrected_vz(double vz, double p, double theta_deg,
                                    double phi_deg, int sector) const -> double {
        if (!m_loaded) return vz;
        return m_corrector->apply(vz, p, theta_deg, phi_deg, sector);
    }

    /// Vz cut for the service's configured (target, polarity).
    ///   * LD2 (or any case where the correction is not loaded): raw vz in
    ///     `[ld2_vz_min, ld2_vz_max]`.
    ///   * Cu  : corrected vz inside the hi-peak (downstream) window.
    ///   * Sn  : corrected vz inside the lo-peak (upstream) window.
    ///   * CuSn / CxC: corrected vz inside either peak window (pass_any).
    [[nodiscard]] auto pass_vz_cut(double vz, double p, double theta_deg,
                                   double phi_deg, int sector) const -> bool {
        if (!m_loaded) return ld2_window_contains(vz);

        const double vz_c = m_corrector->apply(vz, p, theta_deg, phi_deg, sector);
        switch (m_config.target) {
            case Target::Cu:   return m_corrector->pass_hi(vz_c);
            case Target::Sn:   return m_corrector->pass_lo(vz_c);
            case Target::CuSn:
            case Target::CxC:  return m_corrector->pass_any(vz_c);
            default:           return m_corrector->pass_any(vz_c);
        }
    }

    /// True when the pion correction of this charge was found in the params file.
    [[nodiscard]] auto pion_loaded(bool is_piplus) const -> bool {
        return m_loaded && m_pion[is_piplus ? 1 : 0] != nullptr;
    }

    /// Pion per-event correction (pion parameters of this charge; electron
    /// correction if they are missing; raw vz when nothing is loaded).
    [[nodiscard]] auto corrected_vz_pion(double vz, double p, double theta_deg,
                                         double phi_deg, int sector, bool is_piplus) const -> double {
        if (!m_loaded) return vz;
        const auto& c = m_pion[is_piplus ? 1 : 0];
        return c ? c->apply(vz, p, theta_deg, phi_deg, sector)
                 : m_corrector->apply(vz, p, theta_deg, phi_deg, sector);
    }

    /// Pion Vz cut: pion correction + Table 8 windows (same target logic as
    /// pass_vz_cut). Falls back to pass_vz_cut (electron) if the pion
    /// parameters are missing, and to the LD2 window when nothing is loaded.
    [[nodiscard]] auto pass_vz_cut_pion(double vz, double p, double theta_deg,
                                        double phi_deg, int sector, bool is_piplus) const -> bool {
        if (!m_loaded) return ld2_window_contains(vz);
        const auto& c = m_pion[is_piplus ? 1 : 0];
        if (!c) return pass_vz_cut(vz, p, theta_deg, phi_deg, sector);
        const double vz_c = c->apply(vz, p, theta_deg, phi_deg, sector);
        switch (m_config.target) {
            case Target::Cu:   return c->pass_hi(vz_c);
            case Target::Sn:   return c->pass_lo(vz_c);
            case Target::CuSn:
            case Target::CxC:  return c->pass_any(vz_c);
            default:           return c->pass_any(vz_c);
        }
    }

    [[nodiscard]] auto ld2_window_contains(double vz) const -> bool {
        return vz > m_config.ld2_vz_min && vz < m_config.ld2_vz_max;
    }

    [[nodiscard]] static auto variant_key(Target target, Polarity polarity)
        -> std::optional<std::string> {
        std::string t;
        switch (target) {
            case Target::CuSn:
            case Target::Cu:
            case Target::Sn:  t = "cusn"; break;
            case Target::CxC: t = "cxc";  break;
            default:          return std::nullopt;
        }
        const std::string p = (polarity == Polarity::Outbending) ? "outbending" : "inbending";
        return t + "_" + p;
    }

    /// Voigt fit Gaussian-equivalent (mu, sigma_eff) per peak, from the
    /// analysis note's Table:Vz-cuts. Same windows are reused for all
    /// species; pions on CuSn/CxC inherit the electron fit.
    [[nodiscard]] static auto voigt_window(Target target, Polarity polarity) -> VoigtWindow {
        const bool ob = (polarity == Polarity::Outbending);
        switch (target) {
            case Target::CuSn:
            case Target::Cu:
            case Target::Sn:
                return ob ? VoigtWindow{-2.916, 0.370, -7.861, 0.415}
                          : VoigtWindow{-2.812, 0.315, -7.765, 0.321};
            case Target::CxC:
                return ob ? VoigtWindow{-2.906, 0.373, -7.887, 0.395}
                          : VoigtWindow{-2.713, 0.307, -7.670, 0.313};
            default:
                return {0.0, 1.0, 0.0, 1.0};
        }
    }

    /// Pion windows, common note Table 8 (mu, sigma_eff per foil). lo = downstream
    /// foil (less negative, Sn / C2 peak), hi = upstream foil (Cu / C1 peak),
    /// same convention as voigt_window.
    [[nodiscard]] static auto pion_voigt_window(Target target, Polarity polarity, bool is_piplus) -> VoigtWindow {
        const bool ob = (polarity == Polarity::Outbending);
        switch (target) {
            case Target::CuSn:
            case Target::Cu:
            case Target::Sn:
                if (ob) return is_piplus ? VoigtWindow{-2.821, 0.393, -7.782, 0.403}
                                         : VoigtWindow{-2.823, 0.382, -7.777, 0.449};
                return is_piplus ? VoigtWindow{-2.951, 0.401, -7.840, 0.426}
                                 : VoigtWindow{-2.921, 0.425, -7.830, 0.463};
            case Target::CxC:
                if (ob) return is_piplus ? VoigtWindow{-2.802, 0.409, -7.783, 0.364}
                                         : VoigtWindow{-2.805, 0.389, -7.815, 0.412};
                return is_piplus ? VoigtWindow{-2.849, 0.410, -7.799, 0.421}
                                 : VoigtWindow{-2.782, 0.437, -7.744, 0.463};
            default:
                return {0.0, 1.0, 0.0, 1.0};
        }
    }

   private:
    Config m_config;
    std::unique_ptr<VzCorrector> m_corrector;
    std::unique_ptr<VzCorrector> m_pion[2];  // [0] = pi-, [1] = pi+
    bool m_loaded = false;
    std::string m_active_key;
};

}  // namespace framework
