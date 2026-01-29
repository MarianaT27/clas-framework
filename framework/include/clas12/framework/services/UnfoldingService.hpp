#pragma once

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <TH1D.h>
#include <TH2D.h>
#include <TMatrixD.h>
#include <TVectorD.h>
#include <TDecompSVD.h>

namespace clas12::framework {

/**
 * @brief Unfolding method
 */
enum class UnfoldingMethod {
    MatrixInversion,  ///< Simple matrix inversion (not recommended)
    BinByBin,         ///< Bin-by-bin correction factors
    SVD,              ///< Singular Value Decomposition
    Bayesian          ///< D'Agostini iterative Bayesian
};

/**
 * @brief Get string representation of unfolding method
 */
inline auto unfolding_method_to_string(UnfoldingMethod m) -> std::string {
    switch (m) {
        case UnfoldingMethod::MatrixInversion: return "MatrixInversion";
        case UnfoldingMethod::BinByBin: return "BinByBin";
        case UnfoldingMethod::SVD: return "SVD";
        case UnfoldingMethod::Bayesian: return "Bayesian";
    }
    return "Unknown";
}

/**
 * @brief Regularization type
 */
enum class Regularization {
    None,         ///< No regularization
    Curvature,    ///< Minimize curvature of result
    Derivative,   ///< Minimize derivative
    LCurve        ///< L-curve optimization
};

/**
 * @brief Unfolding result
 */
struct UnfoldingResult {
    bool success = false;
    std::string error_message;

    /// Unfolded distribution
    std::unique_ptr<TH1D> unfolded;

    /// Refolded distribution (for validation)
    std::unique_ptr<TH1D> refolded;

    /// Covariance matrix of unfolded result
    TMatrixD covariance;

    /// Correlation matrix
    TMatrixD correlation;

    /// Chi2 of refolded vs measured
    double chi2_refolded = -1.0;
    int ndf = 0;

    /// Regularization strength used
    double regularization_strength = 0.0;

    /// Number of iterations (for Bayesian)
    int n_iterations = 0;

    // ========================================================================
    // Accessor Methods
    // ========================================================================

    /**
     * @brief Get bin content of unfolded histogram
     */
    [[nodiscard]] auto get_bin_content(int bin) const -> double {
        return unfolded ? unfolded->GetBinContent(bin) : 0.0;
    }

    /**
     * @brief Get bin error of unfolded histogram
     */
    [[nodiscard]] auto get_bin_error(int bin) const -> double {
        return unfolded ? unfolded->GetBinError(bin) : 0.0;
    }

    /**
     * @brief Get correlation between two bins
     */
    [[nodiscard]] auto get_correlation(int bin1, int bin2) const -> double {
        if (correlation.GetNrows() > 0) {
            return correlation(bin1 - 1, bin2 - 1);
        }
        return 0.0;
    }

    /**
     * @brief Check if result is valid
     */
    [[nodiscard]] auto is_valid() const -> bool {
        return success && unfolded != nullptr;
    }
};

/**
 * @brief Response matrix configuration
 */
struct ResponseConfig {
    std::unique_ptr<TH1D> true_hist;      ///< True (generator-level) distribution
    std::unique_ptr<TH1D> reco_hist;      ///< Reconstructed distribution
    std::unique_ptr<TH2D> response;       ///< Response matrix (true vs reco)
    TMatrixD response_matrix;             ///< Normalized response matrix
    TMatrixD inv_response;                ///< Inverted response (for matrix inversion)

    int n_bins_true = 0;
    int n_bins_reco = 0;
    double range_min = 0.0;
    double range_max = 1.0;
};

/**
 * @brief Service for detector response unfolding
 *
 * UnfoldingService provides various unfolding methods to correct measured
 * distributions for detector resolution, acceptance, and efficiency effects.
 *
 * Supported methods:
 * - Bin-by-bin correction (simple, for small migrations)
 * - D'Agostini iterative Bayesian unfolding
 * - SVD unfolding with regularization
 * - Simple matrix inversion (educational only)
 *
 * Example usage:
 * @code
 * // Build response matrix from MC
 * m_unfold_svc->build_response("q2", mc_true_q2, mc_reco_q2,
 *                               20, 25, 1.0, 10.0);
 *
 * // Unfold data distribution
 * auto result = m_unfold_svc->unfold_bayesian("q2", h_measured_q2, 4);
 *
 * if (result.success) {
 *     // Use unfolded histogram
 *     result.unfolded->Write("h_unfolded_q2");
 * }
 * @endcode
 */
class UnfoldingService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "UnfoldingService"; }

    explicit UnfoldingService(std::string name = "UnfoldingService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "UnfoldingService initialized");
        Log::info(this->name(), "  Default Bayesian iterations: {}",
                  m_bayesian_iterations.get());
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "UnfoldingService finalized");
        Log::info(this->name(), "  Response matrices configured: {}",
                  m_responses.size());
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Number of iterations for Bayesian unfolding
    Property<int> m_bayesian_iterations{"bayesian_iterations", 4,
                                        "D'Agostini iterations"};

    /// SVD regularization parameter (0 = auto)
    Property<int> m_svd_kreg{"svd_kreg", 0, "SVD regularization parameter"};

    // ========================================================================
    // Response Matrix Setup
    // ========================================================================

    /**
     * @brief Set response matrix from existing histograms
     *
     * @param name Configuration name
     * @param true_hist True (generator-level) distribution
     * @param reco_hist Reconstructed distribution
     * @param response_matrix 2D histogram (true vs reco)
     * @return Reference to this service
     */
    auto set_response(const std::string& name,
                      const TH1D* true_hist,
                      const TH1D* reco_hist,
                      const TH2D* response_matrix) -> UnfoldingService& {
        ResponseConfig config;

        // Clone histograms
        config.true_hist.reset(static_cast<TH1D*>(true_hist->Clone()));
        config.reco_hist.reset(static_cast<TH1D*>(reco_hist->Clone()));
        config.response.reset(static_cast<TH2D*>(response_matrix->Clone()));

        config.n_bins_true = true_hist->GetNbinsX();
        config.n_bins_reco = reco_hist->GetNbinsX();
        config.range_min = true_hist->GetXaxis()->GetXmin();
        config.range_max = true_hist->GetXaxis()->GetXmax();

        // Build normalized response matrix
        build_response_matrix(config);

        m_responses[name] = std::move(config);

        Log::info(this->name(), "Set response '{}': {} true bins x {} reco bins",
                  name, config.n_bins_true, config.n_bins_reco);

        return *this;
    }

    /**
     * @brief Build response matrix from MC sample
     *
     * @param name Configuration name
     * @param true_values MC true values
     * @param reco_values MC reconstructed values
     * @param nbins_true Number of true bins
     * @param nbins_reco Number of reco bins
     * @param range_min Minimum value
     * @param range_max Maximum value
     * @return Reference to this service
     */
    auto build_response(const std::string& name,
                        const std::vector<double>& true_values,
                        const std::vector<double>& reco_values,
                        int nbins_true, int nbins_reco,
                        double range_min, double range_max) -> UnfoldingService& {
        if (true_values.size() != reco_values.size()) {
            Log::error(this->name(), "True and reco vectors must have same size");
            return *this;
        }

        ResponseConfig config;
        config.n_bins_true = nbins_true;
        config.n_bins_reco = nbins_reco;
        config.range_min = range_min;
        config.range_max = range_max;

        // Create histograms
        std::string true_name = name + "_true";
        std::string reco_name = name + "_reco";
        std::string resp_name = name + "_response";

        config.true_hist = std::make_unique<TH1D>(
            true_name.c_str(), "True distribution",
            nbins_true, range_min, range_max);

        config.reco_hist = std::make_unique<TH1D>(
            reco_name.c_str(), "Reco distribution",
            nbins_reco, range_min, range_max);

        config.response = std::make_unique<TH2D>(
            resp_name.c_str(), "Response matrix",
            nbins_true, range_min, range_max,
            nbins_reco, range_min, range_max);

        // Fill histograms
        for (size_t i = 0; i < true_values.size(); ++i) {
            config.true_hist->Fill(true_values[i]);
            config.reco_hist->Fill(reco_values[i]);
            config.response->Fill(true_values[i], reco_values[i]);
        }

        // Build normalized response matrix
        build_response_matrix(config);

        m_responses[name] = std::move(config);

        Log::info(this->name(), "Built response '{}' from {} events",
                  name, true_values.size());

        return *this;
    }

    // ========================================================================
    // Unfolding Methods
    // ========================================================================

    /**
     * @brief Unfold a measured distribution
     *
     * @param name Response configuration name
     * @param measured Measured (data) histogram
     * @param method Unfolding method
     * @return UnfoldingResult with unfolded distribution
     */
    auto unfold(const std::string& name,
                const TH1D* measured,
                UnfoldingMethod method = UnfoldingMethod::Bayesian) -> UnfoldingResult {
        switch (method) {
            case UnfoldingMethod::Bayesian:
                return unfold_bayesian(name, measured, m_bayesian_iterations.get());
            case UnfoldingMethod::SVD:
                return unfold_svd(name, measured, m_svd_kreg.get());
            case UnfoldingMethod::BinByBin:
                return unfold_bin_by_bin(name, measured);
            case UnfoldingMethod::MatrixInversion:
                return unfold_matrix_inversion(name, measured);
        }
        return UnfoldingResult();
    }

    /**
     * @brief D'Agostini iterative Bayesian unfolding
     *
     * @param name Response configuration
     * @param measured Measured histogram
     * @param n_iterations Number of iterations
     * @return UnfoldingResult
     */
    auto unfold_bayesian(const std::string& name,
                         const TH1D* measured,
                         int n_iterations = 4) -> UnfoldingResult {
        UnfoldingResult result;

        auto it = m_responses.find(name);
        if (it == m_responses.end()) {
            result.error_message = "Response configuration not found: " + name;
            return result;
        }

        const auto& config = it->second;
        const int n_true = config.n_bins_true;
        const int n_reco = config.n_bins_reco;

        // Initialize prior as true distribution (or flat)
        std::vector<double> prior(n_true);
        double prior_sum = 0.0;
        for (int t = 0; t < n_true; ++t) {
            prior[t] = config.true_hist->GetBinContent(t + 1);
            prior_sum += prior[t];
        }
        if (prior_sum > 0) {
            for (int t = 0; t < n_true; ++t) {
                prior[t] /= prior_sum;
            }
        } else {
            // Flat prior
            for (int t = 0; t < n_true; ++t) {
                prior[t] = 1.0 / n_true;
            }
        }

        // Get measured data
        std::vector<double> data(n_reco);
        double data_sum = 0.0;
        for (int r = 0; r < n_reco; ++r) {
            data[r] = measured->GetBinContent(r + 1);
            data_sum += data[r];
        }

        // Response matrix P(r|t)
        const TMatrixD& R = config.response_matrix;

        // Iterative Bayesian unfolding
        std::vector<double> unfolded(n_true);

        for (int iter = 0; iter < n_iterations; ++iter) {
            // Compute P(t|r) using Bayes' theorem
            // P(t|r) = P(r|t) * P(t) / sum_t' P(r|t') * P(t')

            // Denominator: sum_t' P(r|t') * P(t')
            std::vector<double> denom(n_reco, 0.0);
            for (int r = 0; r < n_reco; ++r) {
                for (int t = 0; t < n_true; ++t) {
                    denom[r] += R(t, r) * prior[t];
                }
            }

            // Update unfolded distribution
            for (int t = 0; t < n_true; ++t) {
                unfolded[t] = 0.0;
                for (int r = 0; r < n_reco; ++r) {
                    if (denom[r] > 0) {
                        double P_t_given_r = R(t, r) * prior[t] / denom[r];
                        unfolded[t] += P_t_given_r * data[r];
                    }
                }
            }

            // Update prior for next iteration
            double unf_sum = 0.0;
            for (int t = 0; t < n_true; ++t) {
                unf_sum += unfolded[t];
            }
            if (unf_sum > 0) {
                for (int t = 0; t < n_true; ++t) {
                    prior[t] = unfolded[t] / unf_sum;
                }
            }
        }

        // Create output histogram
        result.unfolded = std::make_unique<TH1D>(
            (name + "_unfolded").c_str(), "Unfolded distribution",
            n_true, config.range_min, config.range_max);

        for (int t = 0; t < n_true; ++t) {
            result.unfolded->SetBinContent(t + 1, unfolded[t]);
            // Error estimation (simplified)
            result.unfolded->SetBinError(t + 1, std::sqrt(unfolded[t]));
        }

        // Compute refolded distribution for validation
        result.refolded = refold(result.unfolded.get(), config);

        // Compute chi2
        result.chi2_refolded = 0.0;
        result.ndf = 0;
        for (int r = 0; r < n_reco; ++r) {
            double obs = measured->GetBinContent(r + 1);
            double exp = result.refolded->GetBinContent(r + 1);
            double err = measured->GetBinError(r + 1);
            if (err > 0) {
                result.chi2_refolded += std::pow((obs - exp) / err, 2);
                ++result.ndf;
            }
        }

        result.n_iterations = n_iterations;
        result.success = true;

        Log::info(this->name(), "Bayesian unfolding '{}': {} iterations, chi2/ndf = {:.2f}",
                  name, n_iterations, result.chi2_refolded / std::max(1, result.ndf));

        return result;
    }

    /**
     * @brief SVD unfolding
     *
     * @param name Response configuration
     * @param measured Measured histogram
     * @param k_reg Regularization parameter (number of singular values to use)
     * @return UnfoldingResult
     */
    auto unfold_svd(const std::string& name,
                    const TH1D* measured,
                    int k_reg = 0) -> UnfoldingResult {
        UnfoldingResult result;

        auto it = m_responses.find(name);
        if (it == m_responses.end()) {
            result.error_message = "Response configuration not found: " + name;
            return result;
        }

        const auto& config = it->second;
        const int n_true = config.n_bins_true;
        const int n_reco = config.n_bins_reco;

        // Get response matrix
        TMatrixD R = config.response_matrix;

        // Get measured data as vector
        TVectorD d(n_reco);
        for (int r = 0; r < n_reco; ++r) {
            d(r) = measured->GetBinContent(r + 1);
        }

        // SVD decomposition: R = U * S * V^T
        TDecompSVD svd(R);
        if (!svd.Decompose()) {
            result.error_message = "SVD decomposition failed";
            return result;
        }

        // Get singular values
        TVectorD singular = svd.GetSig();

        // Auto-determine regularization if not specified
        if (k_reg <= 0 || k_reg > singular.GetNrows()) {
            // Use number of significant singular values
            k_reg = 0;
            double max_sv = singular.Max();
            for (int i = 0; i < singular.GetNrows(); ++i) {
                if (singular(i) > 1e-6 * max_sv) {
                    ++k_reg;
                }
            }
        }

        // Regularized inversion
        TVectorD unf = svd.Solve(d);

        // Create output histogram
        result.unfolded = std::make_unique<TH1D>(
            (name + "_unfolded_svd").c_str(), "Unfolded (SVD)",
            n_true, config.range_min, config.range_max);

        for (int t = 0; t < n_true; ++t) {
            result.unfolded->SetBinContent(t + 1, unf(t));
            result.unfolded->SetBinError(t + 1, std::sqrt(std::abs(unf(t))));
        }

        // Compute refolded
        result.refolded = refold(result.unfolded.get(), config);

        result.regularization_strength = k_reg;
        result.success = true;

        Log::info(this->name(), "SVD unfolding '{}': k_reg = {}", name, k_reg);

        return result;
    }

    /**
     * @brief Simple bin-by-bin correction
     *
     * Only valid for small migrations between bins.
     *
     * @param name Response configuration
     * @param measured Measured histogram
     * @return UnfoldingResult
     */
    auto unfold_bin_by_bin(const std::string& name,
                           const TH1D* measured) -> UnfoldingResult {
        UnfoldingResult result;

        auto it = m_responses.find(name);
        if (it == m_responses.end()) {
            result.error_message = "Response configuration not found: " + name;
            return result;
        }

        const auto& config = it->second;
        const int n_true = config.n_bins_true;

        // Compute correction factors: true / reco
        result.unfolded = std::make_unique<TH1D>(
            (name + "_unfolded_bbb").c_str(), "Unfolded (bin-by-bin)",
            n_true, config.range_min, config.range_max);

        for (int i = 0; i < n_true; ++i) {
            double true_val = config.true_hist->GetBinContent(i + 1);
            double reco_val = config.reco_hist->GetBinContent(i + 1);
            double meas_val = measured->GetBinContent(i + 1);
            double meas_err = measured->GetBinError(i + 1);

            double correction = (reco_val > 0) ? true_val / reco_val : 1.0;
            double unfolded_val = meas_val * correction;
            double unfolded_err = meas_err * correction;

            result.unfolded->SetBinContent(i + 1, unfolded_val);
            result.unfolded->SetBinError(i + 1, unfolded_err);
        }

        result.refolded = refold(result.unfolded.get(), config);
        result.success = true;

        return result;
    }

    /**
     * @brief Simple matrix inversion (not recommended for production)
     */
    auto unfold_matrix_inversion(const std::string& name,
                                 const TH1D* measured) -> UnfoldingResult {
        UnfoldingResult result;

        auto it = m_responses.find(name);
        if (it == m_responses.end()) {
            result.error_message = "Response configuration not found: " + name;
            return result;
        }

        const auto& config = it->second;

        // Use pre-computed inverse if available
        if (config.inv_response.GetNrows() == 0) {
            result.error_message = "Response matrix not invertible";
            return result;
        }

        const int n_true = config.n_bins_true;
        const int n_reco = config.n_bins_reco;

        // Get measured data
        TVectorD d(n_reco);
        for (int r = 0; r < n_reco; ++r) {
            d(r) = measured->GetBinContent(r + 1);
        }

        // Unfold: x = R^-1 * d
        TVectorD x(n_true);
        x = config.inv_response * d;

        // Create output
        result.unfolded = std::make_unique<TH1D>(
            (name + "_unfolded_inv").c_str(), "Unfolded (matrix inv)",
            n_true, config.range_min, config.range_max);

        for (int t = 0; t < n_true; ++t) {
            result.unfolded->SetBinContent(t + 1, x(t));
            result.unfolded->SetBinError(t + 1, std::sqrt(std::abs(x(t))));
        }

        result.refolded = refold(result.unfolded.get(), config);
        result.success = true;

        return result;
    }

    // ========================================================================
    // Validation and Diagnostics
    // ========================================================================

    /**
     * @brief Perform closure test using MC
     */
    auto closure_test(const std::string& name,
                      const TH1D* mc_true,
                      UnfoldingMethod method) -> UnfoldingResult {
        auto it = m_responses.find(name);
        if (it == m_responses.end()) {
            UnfoldingResult result;
            result.error_message = "Response configuration not found: " + name;
            return result;
        }

        // Use reco histogram from MC as "measured"
        return unfold(name, it->second.reco_hist.get(), method);
    }

    /**
     * @brief Get purity per bin (diagonal / total in true bin)
     */
    [[nodiscard]] auto get_purity(const std::string& name) const
        -> std::unique_ptr<TH1D> {
        auto it = m_responses.find(name);
        if (it == m_responses.end()) return nullptr;

        const auto& config = it->second;
        const int n = config.n_bins_true;

        auto purity = std::make_unique<TH1D>(
            (name + "_purity").c_str(), "Purity",
            n, config.range_min, config.range_max);

        for (int t = 0; t < n; ++t) {
            double total = 0.0;
            for (int r = 0; r < config.n_bins_reco; ++r) {
                total += config.response->GetBinContent(t + 1, r + 1);
            }
            double diagonal = config.response->GetBinContent(t + 1, t + 1);
            purity->SetBinContent(t + 1, (total > 0) ? diagonal / total : 0.0);
        }

        return purity;
    }

    /**
     * @brief Get stability per bin (diagonal / total in reco bin)
     */
    [[nodiscard]] auto get_stability(const std::string& name) const
        -> std::unique_ptr<TH1D> {
        auto it = m_responses.find(name);
        if (it == m_responses.end()) return nullptr;

        const auto& config = it->second;
        const int n = config.n_bins_reco;

        auto stability = std::make_unique<TH1D>(
            (name + "_stability").c_str(), "Stability",
            n, config.range_min, config.range_max);

        for (int r = 0; r < n; ++r) {
            double total = 0.0;
            for (int t = 0; t < config.n_bins_true; ++t) {
                total += config.response->GetBinContent(t + 1, r + 1);
            }
            double diagonal = config.response->GetBinContent(r + 1, r + 1);
            stability->SetBinContent(r + 1, (total > 0) ? diagonal / total : 0.0);
        }

        return stability;
    }

    /**
     * @brief Get efficiency per true bin
     */
    [[nodiscard]] auto get_efficiency(const std::string& name) const
        -> std::unique_ptr<TH1D> {
        auto it = m_responses.find(name);
        if (it == m_responses.end()) return nullptr;

        const auto& config = it->second;
        const int n = config.n_bins_true;

        auto efficiency = std::make_unique<TH1D>(
            (name + "_efficiency").c_str(), "Efficiency",
            n, config.range_min, config.range_max);

        for (int t = 0; t < n; ++t) {
            double reco = config.reco_hist->GetBinContent(t + 1);
            double truth = config.true_hist->GetBinContent(t + 1);
            efficiency->SetBinContent(t + 1, (truth > 0) ? reco / truth : 0.0);
        }

        return efficiency;
    }

    /**
     * @brief Get response matrix
     */
    [[nodiscard]] auto get_response_matrix(const std::string& name) const
        -> const TH2D* {
        auto it = m_responses.find(name);
        if (it == m_responses.end()) return nullptr;
        return it->second.response.get();
    }

   private:
    std::map<std::string, ResponseConfig> m_responses;

    /**
     * @brief Build normalized response matrix
     */
    auto build_response_matrix(ResponseConfig& config) -> void {
        const int n_true = config.n_bins_true;
        const int n_reco = config.n_bins_reco;

        config.response_matrix.ResizeTo(n_true, n_reco);

        // Normalize: P(r|t) = response(t,r) / sum_r' response(t,r')
        for (int t = 0; t < n_true; ++t) {
            double sum = 0.0;
            for (int r = 0; r < n_reco; ++r) {
                sum += config.response->GetBinContent(t + 1, r + 1);
            }
            for (int r = 0; r < n_reco; ++r) {
                config.response_matrix(t, r) =
                    (sum > 0) ? config.response->GetBinContent(t + 1, r + 1) / sum : 0.0;
            }
        }

        // Try to compute inverse (may fail if singular)
        if (n_true == n_reco) {
            config.inv_response.ResizeTo(n_true, n_reco);
            TDecompSVD svd(config.response_matrix);
            if (svd.Decompose()) {
                config.inv_response = svd.Invert();
            }
        }
    }

    /**
     * @brief Refold unfolded histogram (for validation)
     */
    auto refold(const TH1D* unfolded, const ResponseConfig& config) const
        -> std::unique_ptr<TH1D> {
        const int n_true = config.n_bins_true;
        const int n_reco = config.n_bins_reco;

        auto refolded = std::make_unique<TH1D>(
            "refolded", "Refolded",
            n_reco, config.range_min, config.range_max);

        for (int r = 0; r < n_reco; ++r) {
            double val = 0.0;
            for (int t = 0; t < n_true; ++t) {
                val += config.response_matrix(t, r) * unfolded->GetBinContent(t + 1);
            }
            refolded->SetBinContent(r + 1, val);
        }

        return refolded;
    }
};

}  // namespace clas12::framework
