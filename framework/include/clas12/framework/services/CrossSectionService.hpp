#pragma once

/**
 * @file CrossSectionService.hpp
 * @brief Service for calculating differential and integrated cross-sections
 *
 * Provides complete cross-section calculation framework including:
 * - Acceptance correction from MC
 * - Efficiency corrections
 * - Luminosity normalization
 * - Radiative corrections
 * - Systematic uncertainty propagation
 * - Bin-by-bin and integrated cross-sections
 *
 * Main formula: σ = N / (L × A × ε × ΔX × R)
 * where: N = yield, L = luminosity, A = acceptance,
 *        ε = efficiency, ΔX = bin width, R = radiative correction
 *
 * @author CLAS12 Analysis Framework
 * @date 2024
 *
 * References:
 * - CLAS12 analysis notes on cross-section extraction
 * - HERMES/COMPASS cross-section methodologies
 */

#include "clas12/framework/core/Service.hpp"

#include <TGraphAsymmErrors.h>
#include <TH1D.h>
#include <TH2D.h>

#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

namespace clas12::framework {

// ============================================================================
// Cross-section result structure
// ============================================================================

/**
 * @struct CrossSectionResult
 * @brief Contains the calculated cross-section and all uncertainties
 */
struct CrossSectionResult {
    std::string name;
    std::string variable;
    std::string units;

    // Bin information
    std::vector<double> bin_edges;
    std::vector<double> bin_centers;
    std::vector<double> bin_widths;

    // Cross-section values
    std::vector<double> cross_sections;
    std::vector<double> stat_errors;
    std::vector<double> syst_errors_up;
    std::vector<double> syst_errors_down;

    // Total errors (stat + syst combined in quadrature)
    std::vector<double> total_errors_up;
    std::vector<double> total_errors_down;

    // Integrated cross-section
    double total_cross_section      = 0.0;
    double total_stat_error         = 0.0;
    double total_syst_error         = 0.0;
    double integrated_luminosity    = 0.0;

    // Component histograms (for diagnostics)
    std::vector<double> yields;
    std::vector<double> acceptances;
    std::vector<double> efficiencies;
    std::vector<double> radiative_corrections;

    /**
     * @brief Check if calculation succeeded
     */
    [[nodiscard]] auto is_valid() const -> bool {
        return !cross_sections.empty() && integrated_luminosity > 0.0;
    }

    /**
     * @brief Get number of bins
     */
    [[nodiscard]] auto n_bins() const -> size_t { return cross_sections.size(); }

    /**
     * @brief Get cross-section value for a specific bin
     */
    [[nodiscard]] auto get_bin_cross_section(size_t bin) const -> double {
        return bin < cross_sections.size() ? cross_sections[bin] : 0.0;
    }

    /**
     * @brief Get statistical error for a specific bin
     */
    [[nodiscard]] auto get_bin_stat_error(size_t bin) const -> double {
        return bin < stat_errors.size() ? stat_errors[bin] : 0.0;
    }

    /**
     * @brief Get systematic error for a specific bin (averaged up/down)
     */
    [[nodiscard]] auto get_bin_syst_error(size_t bin) const -> double {
        if (bin >= syst_errors_up.size())
            return 0.0;
        return 0.5 * (syst_errors_up[bin] + syst_errors_down[bin]);
    }

    /**
     * @brief Convert to ROOT TGraphAsymmErrors
     */
    [[nodiscard]] auto to_graph() const -> std::unique_ptr<TGraphAsymmErrors> {
        auto graph = std::make_unique<TGraphAsymmErrors>(static_cast<int>(n_bins()));

        for (size_t i = 0; i < n_bins(); ++i) {
            double x_center    = bin_centers[i];
            double x_half      = bin_widths[i] / 2.0;
            double y           = cross_sections[i];
            double y_err_up    = total_errors_up.empty() ? stat_errors[i] : total_errors_up[i];
            double y_err_down  = total_errors_down.empty() ? stat_errors[i] : total_errors_down[i];

            graph->SetPoint(static_cast<int>(i), x_center, y);
            graph->SetPointError(static_cast<int>(i), x_half, x_half, y_err_down, y_err_up);
        }

        graph->SetName(name.c_str());
        graph->SetTitle((name + ";" + variable + ";" + units).c_str());

        return graph;
    }

    /**
     * @brief Convert to ROOT TH1D (for simpler use cases)
     */
    [[nodiscard]] auto to_histogram() const -> std::unique_ptr<TH1D> {
        if (bin_edges.empty())
            return nullptr;

        auto hist = std::make_unique<TH1D>(
            name.c_str(),
            (name + ";" + variable + ";" + units).c_str(),
            static_cast<int>(n_bins()),
            bin_edges.data());

        for (size_t i = 0; i < n_bins(); ++i) {
            hist->SetBinContent(static_cast<int>(i + 1), cross_sections[i]);
            double err = stat_errors.empty() ? 0.0 : stat_errors[i];
            hist->SetBinError(static_cast<int>(i + 1), err);
        }

        return hist;
    }

    /**
     * @brief Print summary of results
     */
    void print_summary() const {
        fmt::print("Cross-section: {}\n", name);
        fmt::print("  Variable: {} [{}]\n", variable, units);
        fmt::print("  Luminosity: {:.3e} /pb\n", integrated_luminosity);
        fmt::print("  Total: {:.4e} ± {:.4e} (stat) ± {:.4e} (syst) {}\n",
                   total_cross_section, total_stat_error, total_syst_error, units);
        fmt::print("  Bins: {}\n", n_bins());

        for (size_t i = 0; i < n_bins(); ++i) {
            fmt::print("    [{:.3f}, {:.3f}]: {:.4e} ± {:.4e}\n",
                       bin_edges[i], bin_edges[i + 1],
                       cross_sections[i], stat_errors[i]);
        }
    }
};

// ============================================================================
// Measurement configuration
// ============================================================================

/**
 * @struct CrossSectionMeasurement
 * @brief Configuration for a cross-section measurement
 */
struct CrossSectionMeasurement {
    std::string name;
    std::string variable;
    std::string units = "nb";

    // Integrated luminosity [/pb or /fb]
    double luminosity = 0.0;

    // Histogram pointers (non-owning, user manages lifetime)
    const TH1D* yield_hist        = nullptr;
    const TH1D* acceptance_hist   = nullptr;
    const TH1D* efficiency_hist   = nullptr;
    const TH1D* rad_corr_hist     = nullptr;

    // Owned histograms (calculated internally)
    std::unique_ptr<TH1D> owned_acceptance;

    // Systematic variations: map from variation name to yield histogram
    std::map<std::string, const TH1D*> systematic_yields;

    // Acceptance calculation histograms
    const TH1D* reco_mc_hist      = nullptr;
    const TH1D* gen_mc_hist       = nullptr;

    // Branching ratio (if applicable)
    double branching_ratio        = 1.0;
    double branching_ratio_error  = 0.0;

    // Target information
    double target_thickness       = 1.0;  // g/cm²
    double target_density         = 1.0;  // atoms/cm³ or similar

    /**
     * @brief Check if measurement is ready for calculation
     */
    [[nodiscard]] auto is_ready() const -> bool {
        return !name.empty() && yield_hist != nullptr && luminosity > 0.0;
    }
};

// ============================================================================
// CrossSectionService
// ============================================================================

/**
 * @class CrossSectionService
 * @brief Service for calculating differential and integrated cross-sections
 *
 * Provides a complete framework for cross-section extraction including:
 * - Acceptance calculation from MC
 * - Efficiency corrections
 * - Luminosity normalization
 * - Radiative corrections
 * - Systematic uncertainty propagation
 *
 * Example usage:
 * @code
 * m_xsec_svc->define_measurement("q2_xsec", "Q²", "nb/GeV²")
 *           ->set_luminosity("q2_xsec", 1.2e6)  // /pb
 *           ->set_yield("q2_xsec", h_yield)
 *           ->calculate_acceptance("q2_xsec", h_reco_mc, h_gen_mc)
 *           ->set_efficiency("q2_xsec", h_efficiency);
 *
 * auto result = m_xsec_svc->calculate("q2_xsec");
 * auto graph = result.to_graph();
 * @endcode
 */
class CrossSectionService : public Service {
public:
    using Service::Service;

    // ========================================================================
    // Properties
    // ========================================================================

    Property<double> m_beam_energy{"beam_energy", 10.6, "Beam energy [GeV]"};
    Property<double> m_min_acceptance{"min_acceptance", 0.01, "Minimum acceptance threshold"};
    Property<double> m_min_efficiency{"min_efficiency", 0.01, "Minimum efficiency threshold"};
    Property<bool> m_apply_bin_width{"apply_bin_width", true, "Divide by bin width for differential"};
    Property<std::string> m_default_units{"default_units", "nb", "Default cross-section units"};

    // ========================================================================
    // Service lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        log_info("Initializing CrossSectionService");
        log_info("  Beam energy: {} GeV", m_beam_energy.value());
        log_info("  Min acceptance: {}", m_min_acceptance.value());
        log_info("  Min efficiency: {}", m_min_efficiency.value());
        return StatusCode::SUCCESS;
    }

    auto on_finalize() -> StatusCode override {
        log_info("Finalizing CrossSectionService");
        log_info("  Measurements defined: {}", m_measurements.size());
        return StatusCode::SUCCESS;
    }

    // ========================================================================
    // Measurement setup
    // ========================================================================

    /**
     * @brief Define a new cross-section measurement
     * @param name Unique identifier for this measurement
     * @param variable Kinematic variable (e.g., "Q²", "W", "cos θ")
     * @param units Cross-section units (default: "nb")
     */
    auto define_measurement(const std::string& name,
                            const std::string& variable,
                            const std::string& units = "nb") -> CrossSectionService& {
        auto& meas     = m_measurements[name];
        meas.name      = name;
        meas.variable  = variable;
        meas.units     = units;

        log_debug("Defined measurement '{}' for variable '{}' [{}]", name, variable, units);
        return *this;
    }

    /**
     * @brief Set integrated luminosity for measurement
     * @param name Measurement name
     * @param luminosity Integrated luminosity [/pb]
     */
    auto set_luminosity(const std::string& name, double luminosity) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.luminosity = luminosity;
        log_debug("Set luminosity for '{}': {:.3e} /pb", name, luminosity);
        return *this;
    }

    /**
     * @brief Set yield histogram
     * @param name Measurement name
     * @param yield Pointer to yield histogram (not owned)
     */
    auto set_yield(const std::string& name, const TH1D* yield) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.yield_hist = yield;
        log_debug("Set yield histogram for '{}'", name);
        return *this;
    }

    /**
     * @brief Set acceptance histogram directly
     * @param name Measurement name
     * @param acceptance Pointer to acceptance histogram (not owned)
     */
    auto set_acceptance(const std::string& name, const TH1D* acceptance) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.acceptance_hist = acceptance;
        log_debug("Set acceptance histogram for '{}'", name);
        return *this;
    }

    /**
     * @brief Calculate acceptance from MC histograms
     * @param name Measurement name
     * @param reco_mc Reconstructed MC histogram
     * @param gen_mc Generated (truth) MC histogram
     */
    auto calculate_acceptance(const std::string& name,
                              const TH1D* reco_mc,
                              const TH1D* gen_mc) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        if (!reco_mc || !gen_mc) {
            log_error("Invalid MC histograms for acceptance calculation");
            return *this;
        }

        // Store for later use
        it->second.reco_mc_hist = reco_mc;
        it->second.gen_mc_hist  = gen_mc;

        // Create acceptance histogram: A = N_reco / N_gen
        auto acc = std::make_unique<TH1D>(*reco_mc);
        acc->SetName((name + "_acceptance").c_str());
        acc->SetTitle("Acceptance");
        acc->Reset();

        int nbins = reco_mc->GetNbinsX();
        for (int i = 1; i <= nbins; ++i) {
            double n_reco = reco_mc->GetBinContent(i);
            double n_gen  = gen_mc->GetBinContent(i);

            if (n_gen > 0) {
                double acceptance = n_reco / n_gen;
                double error      = std::sqrt(acceptance * (1.0 - acceptance) / n_gen);

                // Clip to valid range
                acceptance = std::clamp(acceptance, 0.0, 1.0);

                acc->SetBinContent(i, acceptance);
                acc->SetBinError(i, error);
            }
        }

        it->second.owned_acceptance  = std::move(acc);
        it->second.acceptance_hist   = it->second.owned_acceptance.get();

        log_debug("Calculated acceptance for '{}' with {} bins", name, nbins);
        return *this;
    }

    /**
     * @brief Set efficiency histogram
     * @param name Measurement name
     * @param efficiency Pointer to efficiency histogram (not owned)
     */
    auto set_efficiency(const std::string& name, const TH1D* efficiency) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.efficiency_hist = efficiency;
        log_debug("Set efficiency histogram for '{}'", name);
        return *this;
    }

    /**
     * @brief Set radiative correction histogram
     * @param name Measurement name
     * @param rad_corr Pointer to radiative correction histogram (not owned)
     */
    auto set_radiative_correction(const std::string& name,
                                  const TH1D* rad_corr) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.rad_corr_hist = rad_corr;
        log_debug("Set radiative correction histogram for '{}'", name);
        return *this;
    }

    /**
     * @brief Set branching ratio for exclusive reactions
     * @param name Measurement name
     * @param br Branching ratio
     * @param br_error Branching ratio uncertainty
     */
    auto set_branching_ratio(const std::string& name,
                             double br, double br_error = 0.0) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.branching_ratio       = br;
        it->second.branching_ratio_error = br_error;
        log_debug("Set branching ratio for '{}': {} ± {}", name, br, br_error);
        return *this;
    }

    /**
     * @brief Add systematic variation yield histogram
     * @param name Measurement name
     * @param variation_name Name of the systematic (e.g., "energy_scale_up")
     * @param yield_hist Yield histogram for this variation
     */
    auto add_systematic_yield(const std::string& name,
                              const std::string& variation_name,
                              const TH1D* yield_hist) -> CrossSectionService& {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return *this;
        }

        it->second.systematic_yields[variation_name] = yield_hist;
        log_debug("Added systematic '{}' to measurement '{}'", variation_name, name);
        return *this;
    }

    // ========================================================================
    // Cross-section calculation
    // ========================================================================

    /**
     * @brief Calculate cross-section for a measurement
     * @param name Measurement name
     * @return Cross-section result with values and uncertainties
     *
     * Formula: σ = N / (L × A × ε × ΔX × R × BR)
     * where:
     *   N = measured yield
     *   L = integrated luminosity
     *   A = acceptance (geometric + kinematic)
     *   ε = reconstruction/selection efficiency
     *   ΔX = bin width (for differential cross-sections)
     *   R = radiative correction factor
     *   BR = branching ratio (for exclusive channels)
     */
    auto calculate(const std::string& name) -> CrossSectionResult {
        CrossSectionResult result;
        result.name = name;

        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            log_error("Measurement '{}' not found", name);
            return result;
        }

        const auto& meas = it->second;

        if (!meas.is_ready()) {
            log_error("Measurement '{}' not ready (missing yield or luminosity)", name);
            return result;
        }

        result.variable             = meas.variable;
        result.units                = meas.units;
        result.integrated_luminosity = meas.luminosity;

        const TH1D* yield = meas.yield_hist;
        int nbins         = yield->GetNbinsX();

        // Extract bin information
        result.bin_edges.reserve(nbins + 1);
        for (int i = 0; i <= nbins; ++i) {
            result.bin_edges.push_back(yield->GetBinLowEdge(i + 1));
        }
        result.bin_edges.back() = yield->GetBinLowEdge(nbins + 1);

        result.bin_centers.reserve(nbins);
        result.bin_widths.reserve(nbins);
        result.cross_sections.reserve(nbins);
        result.stat_errors.reserve(nbins);
        result.yields.reserve(nbins);
        result.acceptances.reserve(nbins);
        result.efficiencies.reserve(nbins);
        result.radiative_corrections.reserve(nbins);

        double total_xsec       = 0.0;
        double total_stat_var   = 0.0;

        for (int i = 1; i <= nbins; ++i) {
            double bin_center = yield->GetBinCenter(i);
            double bin_width  = yield->GetBinWidth(i);
            double n          = yield->GetBinContent(i);
            double n_err      = yield->GetBinError(i);

            result.bin_centers.push_back(bin_center);
            result.bin_widths.push_back(bin_width);
            result.yields.push_back(n);

            // Get correction factors
            double acceptance = get_correction_value(meas.acceptance_hist, i, 1.0);
            double efficiency = get_correction_value(meas.efficiency_hist, i, 1.0);
            double rad_corr   = get_correction_value(meas.rad_corr_hist, i, 1.0);

            result.acceptances.push_back(acceptance);
            result.efficiencies.push_back(efficiency);
            result.radiative_corrections.push_back(rad_corr);

            // Check thresholds
            if (acceptance < m_min_acceptance.value() ||
                efficiency < m_min_efficiency.value()) {
                result.cross_sections.push_back(0.0);
                result.stat_errors.push_back(0.0);
                continue;
            }

            // Calculate cross-section
            double denominator = meas.luminosity * acceptance * efficiency * rad_corr * meas.branching_ratio;

            if (m_apply_bin_width.value()) {
                denominator *= bin_width;
            }

            double xsec = (denominator > 0) ? n / denominator : 0.0;

            // Statistical error (Poisson)
            double stat_err = (denominator > 0 && n > 0) ? std::sqrt(n) / denominator : 0.0;

            result.cross_sections.push_back(xsec);
            result.stat_errors.push_back(stat_err);

            // Accumulate for integrated cross-section
            if (m_apply_bin_width.value()) {
                total_xsec     += xsec * bin_width;
                total_stat_var += stat_err * stat_err * bin_width * bin_width;
            } else {
                total_xsec     += xsec;
                total_stat_var += stat_err * stat_err;
            }
        }

        result.total_cross_section = total_xsec;
        result.total_stat_error    = std::sqrt(total_stat_var);

        log_info("Calculated cross-section '{}': total = {:.4e} ± {:.4e} {}",
                 name, result.total_cross_section, result.total_stat_error, result.units);

        return result;
    }

    /**
     * @brief Calculate cross-section with systematic uncertainties
     * @param name Measurement name
     * @return Cross-section result with statistical and systematic errors
     */
    auto calculate_with_systematics(const std::string& name) -> CrossSectionResult {
        // First calculate nominal
        auto result = calculate(name);
        if (!result.is_valid()) {
            return result;
        }

        auto it = m_measurements.find(name);
        if (it == m_measurements.end() || it->second.systematic_yields.empty()) {
            // No systematics defined, return with stat-only
            result.syst_errors_up   = std::vector<double>(result.n_bins(), 0.0);
            result.syst_errors_down = std::vector<double>(result.n_bins(), 0.0);
            result.total_errors_up  = result.stat_errors;
            result.total_errors_down = result.stat_errors;
            return result;
        }

        const auto& meas = it->second;
        size_t nbins     = result.n_bins();

        // Store variations for each bin
        std::vector<std::vector<double>> variations(nbins);

        // Calculate cross-section for each systematic variation
        for (const auto& [var_name, var_yield] : meas.systematic_yields) {
            if (!var_yield)
                continue;

            // Temporarily replace yield and calculate
            auto saved_yield       = meas.yield_hist;
            const_cast<CrossSectionMeasurement&>(meas).yield_hist = var_yield;

            auto var_result = calculate(name);

            const_cast<CrossSectionMeasurement&>(meas).yield_hist = saved_yield;

            // Store variations
            for (size_t i = 0; i < nbins; ++i) {
                if (i < var_result.cross_sections.size()) {
                    double diff = var_result.cross_sections[i] - result.cross_sections[i];
                    variations[i].push_back(diff);
                }
            }
        }

        // Calculate systematic errors (RMS of variations)
        result.syst_errors_up.resize(nbins);
        result.syst_errors_down.resize(nbins);
        result.total_errors_up.resize(nbins);
        result.total_errors_down.resize(nbins);

        double total_syst_var = 0.0;

        for (size_t i = 0; i < nbins; ++i) {
            if (variations[i].empty()) {
                result.syst_errors_up[i]   = 0.0;
                result.syst_errors_down[i] = 0.0;
            } else {
                // Separate positive and negative variations
                double sum_pos_sq = 0.0;
                double sum_neg_sq = 0.0;

                for (double diff : variations[i]) {
                    if (diff > 0) {
                        sum_pos_sq += diff * diff;
                    } else {
                        sum_neg_sq += diff * diff;
                    }
                }

                result.syst_errors_up[i]   = std::sqrt(sum_pos_sq);
                result.syst_errors_down[i] = std::sqrt(sum_neg_sq);
            }

            // Total error (stat + syst in quadrature)
            double stat = result.stat_errors[i];
            result.total_errors_up[i]   = std::sqrt(stat * stat + result.syst_errors_up[i] * result.syst_errors_up[i]);
            result.total_errors_down[i] = std::sqrt(stat * stat + result.syst_errors_down[i] * result.syst_errors_down[i]);

            // For total systematic, use average
            double avg_syst = 0.5 * (result.syst_errors_up[i] + result.syst_errors_down[i]);
            double bin_width = result.bin_widths[i];
            if (m_apply_bin_width.value()) {
                total_syst_var += avg_syst * avg_syst * bin_width * bin_width;
            } else {
                total_syst_var += avg_syst * avg_syst;
            }
        }

        result.total_syst_error = std::sqrt(total_syst_var);

        log_info("Calculated cross-section '{}' with systematics: {:.4e} ± {:.4e} (stat) ± {:.4e} (syst) {}",
                 name, result.total_cross_section, result.total_stat_error,
                 result.total_syst_error, result.units);

        return result;
    }

    // ========================================================================
    // Convenience methods
    // ========================================================================

    /**
     * @brief Calculate flux-averaged cross-section
     * @param name Measurement name
     * @param flux_hist Photon/electron flux histogram
     * @return Cross-section result
     */
    auto calculate_flux_averaged(const std::string& name,
                                 const TH1D* flux_hist) -> CrossSectionResult {
        // Store flux-weighted yield: σ = Σ(N_i / Φ_i) / Σ(1/Φ_i)
        auto result = calculate(name);
        if (!result.is_valid() || !flux_hist) {
            return result;
        }

        // Apply flux weighting
        for (size_t i = 0; i < result.n_bins(); ++i) {
            int bin       = static_cast<int>(i + 1);
            double flux   = flux_hist->GetBinContent(bin);
            if (flux > 0) {
                result.cross_sections[i] /= flux;
                result.stat_errors[i]    /= flux;
            }
        }

        return result;
    }

    /**
     * @brief Get acceptance for a specific measurement
     */
    [[nodiscard]] auto get_acceptance_hist(const std::string& name) const
        -> const TH1D* {
        auto it = m_measurements.find(name);
        if (it == m_measurements.end()) {
            return nullptr;
        }
        return it->second.acceptance_hist;
    }

    /**
     * @brief Get purity histogram (N_signal / N_total in each bin)
     * @param name Measurement name
     * @param signal_mc Signal MC histogram (truth-matched)
     * @param total_mc Total MC histogram (all reconstructed)
     */
    [[nodiscard]] auto calculate_purity(const std::string& name,
                                        const TH1D* signal_mc,
                                        const TH1D* total_mc) const
        -> std::unique_ptr<TH1D> {
        if (!signal_mc || !total_mc) {
            return nullptr;
        }

        auto purity = std::make_unique<TH1D>(*signal_mc);
        purity->SetName((name + "_purity").c_str());
        purity->SetTitle("Purity;Bin;Purity");
        purity->Reset();

        int nbins = signal_mc->GetNbinsX();
        for (int i = 1; i <= nbins; ++i) {
            double n_sig   = signal_mc->GetBinContent(i);
            double n_total = total_mc->GetBinContent(i);

            if (n_total > 0) {
                double pur = n_sig / n_total;
                double err = std::sqrt(pur * (1.0 - pur) / n_total);
                purity->SetBinContent(i, pur);
                purity->SetBinError(i, err);
            }
        }

        return purity;
    }

    /**
     * @brief Calculate bin migration matrix
     * @param name Measurement name
     * @param response_2d 2D histogram (true vs reco)
     */
    [[nodiscard]] auto calculate_migration(const std::string& /*name*/,
                                           const TH2D* response_2d) const
        -> std::unique_ptr<TH2D> {
        if (!response_2d) {
            return nullptr;
        }

        // Normalize each true bin column to get migration probabilities
        auto migration = std::make_unique<TH2D>(*response_2d);
        migration->SetName("migration_matrix");
        migration->SetTitle("Migration Matrix;True;Reconstructed");

        int nbins_x = response_2d->GetNbinsX();
        int nbins_y = response_2d->GetNbinsY();

        for (int ix = 1; ix <= nbins_x; ++ix) {
            double col_sum = 0.0;
            for (int iy = 1; iy <= nbins_y; ++iy) {
                col_sum += response_2d->GetBinContent(ix, iy);
            }

            if (col_sum > 0) {
                for (int iy = 1; iy <= nbins_y; ++iy) {
                    double val = response_2d->GetBinContent(ix, iy) / col_sum;
                    migration->SetBinContent(ix, iy, val);
                }
            }
        }

        return migration;
    }

    /**
     * @brief Check if a measurement exists
     */
    [[nodiscard]] auto has_measurement(const std::string& name) const -> bool {
        return m_measurements.find(name) != m_measurements.end();
    }

    /**
     * @brief Get list of defined measurements
     */
    [[nodiscard]] auto get_measurement_names() const -> std::vector<std::string> {
        std::vector<std::string> names;
        names.reserve(m_measurements.size());
        for (const auto& [name, _] : m_measurements) {
            names.push_back(name);
        }
        return names;
    }

private:
    std::map<std::string, CrossSectionMeasurement> m_measurements;

    /**
     * @brief Get correction value from histogram, or default if null
     */
    [[nodiscard]] auto get_correction_value(const TH1D* hist, int bin, double default_val) const
        -> double {
        if (!hist) {
            return default_val;
        }
        double val = hist->GetBinContent(bin);
        return (val > 0) ? val : default_val;
    }
};

}  // namespace clas12::framework
