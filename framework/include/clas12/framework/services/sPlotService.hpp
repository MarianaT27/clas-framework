#pragma once

#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <TF1.h>
#include <TH1D.h>
#include <TMinuit.h>
#include <Eigen/Dense>

namespace clas12::framework {

/**
 * @brief PDF component for sPlot fit
 */
struct sPlotPDF {
    std::string name;
    bool is_signal = false;

    // PDF function: returns probability density at x
    std::function<double(double)> pdf;

    // Fitted yield
    double yield = 0.0;
    double yield_error = 0.0;

    // Parameter bounds for yield
    double yield_min = 0.0;
    double yield_max = 1e6;
};

/**
 * @brief Result of sPlot calculation
 */
struct sPlotResult {
    bool success = false;
    std::string error_message;

    /// Component yields from fit
    std::map<std::string, double> yields;
    std::map<std::string, double> yield_errors;

    /// sWeights for each event (indexed by event number)
    std::map<std::string, std::vector<double>> sweights;

    /// Covariance matrix of yields
    Eigen::MatrixXd covariance;

    /// Fit quality
    double chi2 = -1.0;
    int ndf = 0;

    // ========================================================================
    // Accessor Methods
    // ========================================================================

    /**
     * @brief Get signal yield
     */
    [[nodiscard]] auto signal_yield() const -> double {
        for (const auto& [name, yield] : yields) {
            if (name.find("signal") != std::string::npos ||
                name.find("sig") != std::string::npos) {
                return yield;
            }
        }
        // Return first yield if no "signal" found
        if (!yields.empty()) {
            return yields.begin()->second;
        }
        return 0.0;
    }

    /**
     * @brief Get signal yield error
     */
    [[nodiscard]] auto signal_yield_error() const -> double {
        for (const auto& [name, error] : yield_errors) {
            if (name.find("signal") != std::string::npos ||
                name.find("sig") != std::string::npos) {
                return error;
            }
        }
        if (!yield_errors.empty()) {
            return yield_errors.begin()->second;
        }
        return 0.0;
    }

    /**
     * @brief Get sWeight for specific component and event
     */
    [[nodiscard]] auto get_sweight(const std::string& component, size_t event_idx) const
        -> double {
        auto it = sweights.find(component);
        if (it == sweights.end()) return 0.0;
        if (event_idx >= it->second.size()) return 0.0;
        return it->second[event_idx];
    }

    /**
     * @brief Get signal sWeight for event
     */
    [[nodiscard]] auto signal_sweight(size_t event_idx) const -> double {
        for (const auto& [name, weights] : sweights) {
            if (name.find("signal") != std::string::npos ||
                name.find("sig") != std::string::npos) {
                if (event_idx < weights.size()) {
                    return weights[event_idx];
                }
            }
        }
        return 0.0;
    }
};

/**
 * @brief Configuration for an sPlot analysis
 */
struct sPlotConfig {
    std::string name;
    std::string discriminating_var;
    double var_min = 0.0;
    double var_max = 1.0;

    std::vector<sPlotPDF> components;
    sPlotResult result;
};

/**
 * @brief Service for sPlot statistical background subtraction
 *
 * sPlotService implements the sPlot technique for extracting signal
 * distributions by computing per-event weights (sWeights) from a
 * discriminating variable fit.
 *
 * The sPlot technique allows extraction of signal properties without
 * cutting on the discriminating variable (typically invariant mass).
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_splot_svc = &require_service<sPlotService>("sPlotService");
 *
 * // Configure sPlot for Lambda mass
 * m_splot_svc->create_splot("lambda", "mass", 1.08, 1.18)
 *            .add_gaussian_signal("lambda", "signal", 1.1157, 0.002)
 *            .add_polynomial_background("lambda", "background", 2);
 *
 * // After event loop, compute sWeights
 * auto result = m_splot_svc->compute_sweights("lambda", masses);
 *
 * if (result.success) {
 *     // Fill background-subtracted histogram
 *     m_splot_svc->fill_with_sweights("lambda", "signal",
 *                                      h_cos_theta, cos_theta_values);
 * }
 * @endcode
 */
class sPlotService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "sPlotService"; }

    explicit sPlotService(std::string name = "sPlotService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "sPlotService initialized");
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "sPlotService finalized");
        Log::info(this->name(), "  Configurations created: {}", m_configs.size());
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Fit strategy (0=standard, 1=improve, 2=improve+minos)
    Property<int> m_fit_strategy{"fit_strategy", 1, "Fit minimizer strategy"};

    /// Use MINOS for error estimation
    Property<bool> m_use_minos{"use_minos", false, "Use MINOS for errors"};

    // ========================================================================
    // sPlot Configuration
    // ========================================================================

    /**
     * @brief Create a new sPlot configuration
     *
     * @param name Configuration name
     * @param discriminating_var Variable name (for documentation)
     * @param var_min Minimum value
     * @param var_max Maximum value
     * @return Reference to this service for chaining
     */
    auto create_splot(const std::string& name,
                      const std::string& discriminating_var,
                      double var_min, double var_max) -> sPlotService& {
        sPlotConfig config;
        config.name = name;
        config.discriminating_var = discriminating_var;
        config.var_min = var_min;
        config.var_max = var_max;
        m_configs[name] = config;
        return *this;
    }

    /**
     * @brief Add Gaussian signal component
     */
    auto add_gaussian_signal(const std::string& splot_name,
                             const std::string& component_name,
                             double mean_init, double sigma_init) -> sPlotService& {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return *this;
        }

        sPlotPDF pdf;
        pdf.name = component_name;
        pdf.is_signal = true;

        // Store parameters for fit
        double mean = mean_init;
        double sigma = sigma_init;
        double x_min = it->second.var_min;
        double x_max = it->second.var_max;

        pdf.pdf = [mean, sigma, x_min, x_max](double x) -> double {
            // Normalized Gaussian
            double norm = 1.0 / (sigma * std::sqrt(2.0 * M_PI));
            double arg = (x - mean) / sigma;
            double gauss = norm * std::exp(-0.5 * arg * arg);

            // Account for range normalization (approximate)
            return gauss;
        };

        it->second.components.push_back(pdf);
        return *this;
    }

    /**
     * @brief Add Crystal Ball signal component
     */
    auto add_crystal_ball_signal(const std::string& splot_name,
                                 const std::string& component_name,
                                 double mean, double sigma,
                                 double alpha, double n) -> sPlotService& {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return *this;
        }

        sPlotPDF pdf;
        pdf.name = component_name;
        pdf.is_signal = true;

        pdf.pdf = [mean, sigma, alpha, n](double x) -> double {
            double t = (x - mean) / sigma;
            if (alpha < 0) t = -t;

            double abs_alpha = std::abs(alpha);

            if (t >= -abs_alpha) {
                return std::exp(-0.5 * t * t);
            } else {
                double A = std::pow(n / abs_alpha, n) * std::exp(-0.5 * abs_alpha * abs_alpha);
                double B = n / abs_alpha - abs_alpha;
                return A / std::pow(B - t, n);
            }
        };

        it->second.components.push_back(pdf);
        return *this;
    }

    /**
     * @brief Add Double Gaussian signal component
     */
    auto add_double_gaussian_signal(const std::string& splot_name,
                                    const std::string& component_name,
                                    double mean, double sigma1, double sigma2,
                                    double fraction) -> sPlotService& {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return *this;
        }

        sPlotPDF pdf;
        pdf.name = component_name;
        pdf.is_signal = true;

        pdf.pdf = [mean, sigma1, sigma2, fraction](double x) -> double {
            double g1 = std::exp(-0.5 * std::pow((x - mean) / sigma1, 2)) /
                        (sigma1 * std::sqrt(2.0 * M_PI));
            double g2 = std::exp(-0.5 * std::pow((x - mean) / sigma2, 2)) /
                        (sigma2 * std::sqrt(2.0 * M_PI));
            return fraction * g1 + (1.0 - fraction) * g2;
        };

        it->second.components.push_back(pdf);
        return *this;
    }

    /**
     * @brief Add polynomial background
     */
    auto add_polynomial_background(const std::string& splot_name,
                                   const std::string& component_name,
                                   int order) -> sPlotService& {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return *this;
        }

        sPlotPDF pdf;
        pdf.name = component_name;
        pdf.is_signal = false;

        double x_min = it->second.var_min;
        double x_max = it->second.var_max;

        // Simple flat background for now (polynomial order 0)
        // Higher orders would need parameter fitting
        pdf.pdf = [x_min, x_max, order](double x) -> double {
            // Uniform background normalized to range
            return 1.0 / (x_max - x_min);
        };

        it->second.components.push_back(pdf);
        return *this;
    }

    /**
     * @brief Add exponential background
     */
    auto add_exponential_background(const std::string& splot_name,
                                    const std::string& component_name,
                                    double tau_init) -> sPlotService& {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return *this;
        }

        sPlotPDF pdf;
        pdf.name = component_name;
        pdf.is_signal = false;

        double x_min = it->second.var_min;
        double x_max = it->second.var_max;
        double tau = tau_init;

        pdf.pdf = [x_min, x_max, tau](double x) -> double {
            // Normalized exponential
            double norm = tau * (std::exp(-x_min / tau) - std::exp(-x_max / tau));
            return std::exp(-x / tau) / norm;
        };

        it->second.components.push_back(pdf);
        return *this;
    }

    // ========================================================================
    // sWeight Computation
    // ========================================================================

    /**
     * @brief Perform sPlot fit and compute sWeights
     *
     * @param splot_name Configuration name
     * @param data Vector of discriminating variable values
     * @return sPlotResult with yields and sWeights
     */
    auto compute_sweights(const std::string& splot_name,
                          const std::vector<double>& data) -> sPlotResult {
        sPlotResult result;

        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            result.error_message = "sPlot configuration not found: " + splot_name;
            return result;
        }

        auto& config = it->second;

        if (config.components.empty()) {
            result.error_message = "No PDF components defined";
            return result;
        }

        if (data.empty()) {
            result.error_message = "No data provided";
            return result;
        }

        const int n_events = static_cast<int>(data.size());
        const int n_components = static_cast<int>(config.components.size());

        // Step 1: Extended maximum likelihood fit for yields
        // Simplified: use method of moments / histogram fitting

        // Build histogram for fitting
        const int n_bins = 50;
        std::vector<double> hist_counts(n_bins, 0.0);
        double bin_width = (config.var_max - config.var_min) / n_bins;

        for (double x : data) {
            int bin = static_cast<int>((x - config.var_min) / bin_width);
            if (bin >= 0 && bin < n_bins) {
                hist_counts[bin] += 1.0;
            }
        }

        // Simple yield estimation using least squares
        // For each component, estimate yield by comparing PDF shape to data

        // Matrix of PDF values at bin centers
        Eigen::MatrixXd A(n_bins, n_components);
        Eigen::VectorXd b(n_bins);

        for (int i = 0; i < n_bins; ++i) {
            double x = config.var_min + (i + 0.5) * bin_width;
            b(i) = hist_counts[i];

            for (int j = 0; j < n_components; ++j) {
                A(i, j) = config.components[j].pdf(x) * bin_width * n_events;
            }
        }

        // Solve for yields: A * yields = b (least squares)
        Eigen::VectorXd yields = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);

        // Store yields
        for (int j = 0; j < n_components; ++j) {
            double y = std::max(0.0, yields(j));  // Non-negative
            config.components[j].yield = y;
            result.yields[config.components[j].name] = y;

            // Estimate error (simplified)
            result.yield_errors[config.components[j].name] = std::sqrt(y);
        }

        // Step 2: Compute sWeights
        // sWeight_n(x) = sum_j V_nj * f_j(x) / sum_k N_k * f_k(x)
        // where V is the covariance matrix of yields

        // Compute covariance matrix (simplified: diagonal)
        result.covariance = Eigen::MatrixXd::Zero(n_components, n_components);
        for (int j = 0; j < n_components; ++j) {
            result.covariance(j, j) = config.components[j].yield;
        }

        // Invert covariance (element-wise for diagonal)
        Eigen::MatrixXd V_inv = Eigen::MatrixXd::Zero(n_components, n_components);
        for (int j = 0; j < n_components; ++j) {
            if (result.covariance(j, j) > 0) {
                V_inv(j, j) = 1.0 / result.covariance(j, j);
            }
        }

        // Compute sWeights for each event
        for (int j = 0; j < n_components; ++j) {
            result.sweights[config.components[j].name].resize(n_events);
        }

        for (int i = 0; i < n_events; ++i) {
            double x = data[i];

            // Compute denominator: sum_k N_k * f_k(x)
            double denom = 0.0;
            for (int k = 0; k < n_components; ++k) {
                denom += config.components[k].yield * config.components[k].pdf(x);
            }

            if (denom < 1e-10) {
                // Avoid division by zero
                for (int j = 0; j < n_components; ++j) {
                    result.sweights[config.components[j].name][i] = 0.0;
                }
                continue;
            }

            // Compute sWeight for each component
            for (int n = 0; n < n_components; ++n) {
                double sweight = 0.0;
                for (int j = 0; j < n_components; ++j) {
                    sweight += V_inv(n, j) * config.components[j].pdf(x);
                }
                sweight /= denom;
                result.sweights[config.components[n].name][i] = sweight;
            }
        }

        // Store result in config
        config.result = result;
        result.success = true;

        Log::info(this->name(), "sPlot '{}' computed:", splot_name);
        for (const auto& [name, yield] : result.yields) {
            Log::info(this->name(), "  {} yield: {:.1f} +/- {:.1f}",
                      name, yield, result.yield_errors[name]);
        }

        return result;
    }

    // ========================================================================
    // sWeight Application
    // ========================================================================

    /**
     * @brief Get signal sWeight for event
     *
     * @param splot_name Configuration name
     * @param event_idx Event index
     * @return Signal sWeight
     */
    [[nodiscard]] auto signal_weight(const std::string& splot_name,
                                     size_t event_idx) const -> double {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) return 0.0;

        return it->second.result.signal_sweight(event_idx);
    }

    /**
     * @brief Get sWeight for specific component
     */
    [[nodiscard]] auto component_weight(const std::string& splot_name,
                                        const std::string& component,
                                        size_t event_idx) const -> double {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) return 0.0;

        return it->second.result.get_sweight(component, event_idx);
    }

    /**
     * @brief Fill histogram with sWeights applied
     *
     * @param splot_name sPlot configuration
     * @param component Component to weight by
     * @param histogram Histogram to fill
     * @param values Variable values to fill
     */
    auto fill_with_sweights(const std::string& splot_name,
                            const std::string& component,
                            TH1D* histogram,
                            const std::vector<double>& values) -> void {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) {
            Log::error(this->name(), "sPlot '{}' not found", splot_name);
            return;
        }

        if (!histogram) {
            Log::error(this->name(), "Null histogram");
            return;
        }

        const auto& sweights = it->second.result.sweights;
        auto sw_it = sweights.find(component);
        if (sw_it == sweights.end()) {
            Log::error(this->name(), "Component '{}' not found in sPlot", component);
            return;
        }

        const auto& weights = sw_it->second;

        for (size_t i = 0; i < values.size() && i < weights.size(); ++i) {
            histogram->Fill(values[i], weights[i]);
        }
    }

    // ========================================================================
    // Diagnostics
    // ========================================================================

    /**
     * @brief Get fit result for diagnostics
     */
    [[nodiscard]] auto get_result(const std::string& splot_name) const
        -> const sPlotResult* {
        auto it = m_configs.find(splot_name);
        if (it == m_configs.end()) return nullptr;
        return &it->second.result;
    }

    /**
     * @brief Check if sPlot configuration exists
     */
    [[nodiscard]] auto has_splot(const std::string& name) const -> bool {
        return m_configs.count(name) > 0;
    }

   private:
    std::map<std::string, sPlotConfig> m_configs;
};

}  // namespace clas12::framework
