#pragma once

#include <clas12/Target.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/StatusCode.hpp>

#include <array>
#include <string>
#include <unordered_map>

namespace framework {

/**
 * @brief Polynomial coefficients for sampling fraction parameterization
 */
struct PolynomialCoefficients {
    double a, b, c, d;

    PolynomialCoefficients(double a = 0.0, double b = 0.0, double c = 0.0, double d = 0.0)
        : a(a), b(b), c(c), d(d) {}

    // Evaluate polynomial: a + b*x + c*x^2 + d*x^3
    [[nodiscard]] double evaluate(double x) const {
        return a + b * x + c * x * x + d * x * x * x;
    }
};

/**
 * @brief Parameters for a single sector's sampling fraction
 */
struct SectorSFParameters {
    PolynomialCoefficients mu;
    PolynomialCoefficients sigma;

    SectorSFParameters() = default;

    SectorSFParameters(const PolynomialCoefficients& mu_coeffs,
                       const PolynomialCoefficients& sigma_coeffs)
        : mu(mu_coeffs), sigma(sigma_coeffs) {}
};

/**
 * @brief Service for sampling fraction cuts in CLAS12 electron analysis
 *
 * This service provides polarity-dependent sampling fraction parameterizations
 * for electron identification. It is implemented as a singleton-like service
 * that can be shared across algorithms.
 *
 * The sampling fraction is parameterized as:
 *   SF = μ(p) ± n·σ(p)
 *
 * where μ and σ are 3rd order polynomials in momentum p:
 *   μ(p) = a + b*p + c*p² + d*p³
 *   σ(p) = a + b*p + c*p² + d*p³
 *
 * Parameters are target-averaged and polarity-dependent (OB/IB).
 *
 * Usage:
 * @code
 * auto sf_svc = service<framework::SamplingFractionService>("SamplingFractionService");
 *
 * double mu = sf_svc->evaluate_mu("OB", sector, momentum);
 * double sigma = sf_svc->evaluate_sigma("OB", sector, momentum);
 *
 * // Or use convenience methods
 * bool passes = sf_svc->passes_cut(SF, momentum, sector,
 *                                   Polarity::Outbending);
 * @endcode
 */
class SamplingFractionService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "SamplingFractionService"; }

    static constexpr int NUM_SECTORS = 6;
    static constexpr double DEFAULT_N_SIGMA = 3.5;

    // ============================================================================
    // Service Interface
    // ============================================================================

    explicit SamplingFractionService(std::string name = "SamplingFractionService")
        : Service(std::move(name)) {
        initialize_data();
    }

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    // ============================================================================
    // Configuration
    // ============================================================================

    void set_n_sigma(double n_sigma) { m_n_sigma = n_sigma; }

    [[nodiscard]] auto n_sigma() const -> double { return m_n_sigma; }

    // ============================================================================
    // Parameter Access
    // ============================================================================

    /**
     * @brief Get parameters for a specific polarity and sector
     */
    [[nodiscard]] auto get_parameters(const std::string& polarity, int sector) const
        -> const SectorSFParameters& {
        return m_data.at(polarity)[sector - 1];
    }

    /**
     * @brief Evaluate μ for given polarity, sector, and momentum
     */
    [[nodiscard]] auto evaluate_mu(const std::string& polarity, int sector, double p) const
        -> double {
        return get_parameters(polarity, sector).mu.evaluate(p);
    }

    /**
     * @brief Evaluate σ for given polarity, sector, and momentum
     */
    [[nodiscard]] auto evaluate_sigma(const std::string& polarity, int sector, double p) const
        -> double {
        return get_parameters(polarity, sector).sigma.evaluate(p);
    }

    /**
     * @brief Evaluate μ + n·σ (upper cut)
     */
    [[nodiscard]] auto evaluate_plus_cut(const std::string& polarity, int sector, double p, double n) const
        -> double {
        const auto& params = get_parameters(polarity, sector);
        return params.mu.evaluate(p) + n * params.sigma.evaluate(p);
    }

    /**
     * @brief Evaluate μ - n·σ (lower cut)
     */
    [[nodiscard]] auto evaluate_minus_cut(const std::string& polarity, int sector, double p, double n) const
        -> double {
        const auto& params = get_parameters(polarity, sector);
        return params.mu.evaluate(p) - n * params.sigma.evaluate(p);
    }

    // ============================================================================
    // Convenience Methods with Polarity
    // ============================================================================

    /**
     * @brief Check if sampling fraction passes the cut
     */
    [[nodiscard]] auto passes_cut(double SF, double p, int sector, Polarity polarity) const
        -> bool {
        return passes_cut(SF, p, sector, polarity, m_n_sigma);
    }

    /**
     * @brief Check if sampling fraction passes the cut with custom n_sigma
     */
    [[nodiscard]] auto passes_cut(double SF, double p, int sector, Polarity polarity, double n_sigma) const
        -> bool {
        if (sector < 1 || sector > NUM_SECTORS) return false;

        std::string polarity_str = (polarity == Polarity::Outbending) ? "OB" : "IB";

        double cut_low = evaluate_minus_cut(polarity_str, sector, p, n_sigma);
        double cut_high = evaluate_plus_cut(polarity_str, sector, p, n_sigma);

        return cut_low < SF && SF < cut_high;
    }

    /**
     * @brief Get μ and σ values
     */
    [[nodiscard]] auto get_mean_sigma(double p, int sector, Polarity polarity) const -> std::pair<double, double> {
        std::string polarity_str = (polarity == Polarity::Outbending) ? "OB" : "IB";

        double mu = evaluate_mu(polarity_str, sector, p);
        double sigma = evaluate_sigma(polarity_str, sector, p);

        return {mu, sigma};
    }

    /**
     * @brief Get μ value using Polarity
     */
    [[nodiscard]] auto get_mu(double p, int sector, Polarity polarity) const
        -> double {
        std::string polarity_str = (polarity == Polarity::Outbending) ? "OB" : "IB";
        return evaluate_mu(polarity_str, sector, p);
    }

    /**
     * @brief Get σ value using Polarity
     */
    [[nodiscard]] auto get_sigma(double p, int sector, Polarity polarity) const
        -> double {
        std::string polarity_str = (polarity == Polarity::Outbending) ? "OB" : "IB";
        return evaluate_sigma(polarity_str, sector, p);
    }

   private:
    std::unordered_map<std::string, std::array<SectorSFParameters, NUM_SECTORS>> m_data;
    double m_n_sigma = DEFAULT_N_SIGMA;

    void initialize_data() {
        // Target-averaged Outbending parameters (6 sectors)
        m_data["OB"] = {{SectorSFParameters(
                             {2.300896E-01, 4.862074E-03, -6.441645E-04, 1.247456E-05},
                             {2.226749E-02, -3.186742E-03, 3.087387E-04, -1.247691E-05}),
                         SectorSFParameters(
                             {2.286452E-01, 4.985634E-03, -3.668113E-04, -1.110160E-05},
                             {2.150934E-02, -2.549354E-03, 1.855833E-04, -4.394923E-06}),
                         SectorSFParameters(
                             {2.258053E-01, 9.073840E-03, -1.240745E-03, 3.579321E-05},
                             {2.399538E-02, -4.208668E-03, 4.503779E-04, -1.783519E-05}),
                         SectorSFParameters(
                             {2.281016E-01, 6.073295E-03, -5.916479E-04, 2.569914E-06},
                             {2.217527E-02, -2.788589E-03, 2.266746E-04, -7.717066E-06}),
                         SectorSFParameters(
                             {2.302059E-01, 3.020237E-03, -4.307581E-05, -2.216133E-05},
                             {2.277357E-02, -3.941980E-03, 4.455085E-04, -1.912118E-05}),
                         SectorSFParameters(
                             {2.262583E-01, 6.849767E-03, -8.027577E-04, 1.288497E-05},
                             {2.253050E-02, -3.334553E-03, 3.374127E-04, -1.413483E-05})}};

        // Target-averaged Inbending parameters (6 sectors).
        // From the IB check (M. Tenorio, mini note "Selection Summary and Cut Efficiencies for
        // Inbending Runs", Table tab:sf-params-ib-new, "Average" rows): Gaussian fits of SF in
        // momentum slices in a fixed +-0.02 window around the previous IB mean, PCAL fiducial cut
        // applied, cubic fits over 1.8 <= p <= 9.0 GeV (extrapolated above), runs 18335 (LD2),
        // 18344 (CxC), 18384 (CuSn). Replaces the IB values of the common note (Table 14 there):
        //   S1 mu {2.311088E-01, 4.309933E-03, -5.451627E-04,  4.965823E-06} sigma {2.235239E-02, -3.117249E-03, 2.887205E-04, -1.130317E-05}
        //   S2 mu {2.304337E-01, 3.585716E-03, -1.672223E-04, -7.317175E-06} sigma {2.170118E-02, -2.428037E-03, 1.302147E-04, -2.503248E-09}
        //   S3 mu {2.269123E-01, 8.797582E-03, -1.171416E-03,  3.051342E-05} sigma {2.423467E-02, -4.301928E-03, 4.742880E-04, -1.983022E-05}
        //   S4 mu {2.288870E-01, 6.786520E-03, -8.022602E-04,  1.557251E-05} sigma {2.313626E-02, -3.260271E-03, 3.018634E-04, -1.156770E-05}
        //   S5 mu {2.288645E-01, 4.637362E-03, -3.662892E-04, -4.233509E-06} sigma {2.310895E-02, -3.892876E-03, 4.115173E-04, -1.656769E-05}
        //   S6 mu {2.272093E-01, 6.581981E-03, -8.197177E-04,  1.632178E-05} sigma {2.321748E-02, -3.665685E-03, 3.804588E-04, -1.587616E-05}
        m_data["IB"] = {{SectorSFParameters(
                             {2.172761E-01, 9.874863E-03, -1.460141E-03, 4.856991E-05},
                             {2.970359E-02, -6.146500E-03, 8.519948E-04, -4.106624E-05}),
                         SectorSFParameters(
                             {2.295669E-01, 5.180274E-03, -9.473454E-06, -8.477999E-05},
                             {2.903474E-02, -5.861235E-03, 7.084581E-04, -1.055444E-05}),
                         SectorSFParameters(
                             {2.287942E-01, 6.720455E-03, -6.166277E-04, -2.783523E-05},
                             {2.597826E-02, -4.096651E-03, 3.504637E-04, 4.854673E-07}),
                         SectorSFParameters(
                             {2.279671E-01, 7.922469E-03, -8.822416E-04, 6.664547E-07},
                             {2.932152E-02, -5.793094E-03, 7.024527E-04, -2.655757E-05}),
                         SectorSFParameters(
                             {2.293399E-01, 6.470559E-03, -7.222006E-04, 5.215601E-06},
                             {2.609526E-02, -4.366779E-03, 4.176203E-04, -9.356880E-06}),
                         SectorSFParameters(
                             {2.307398E-01, 5.277840E-03, -3.128345E-04, -5.064927E-05},
                             {2.886612E-02, -5.828894E-03, 6.541773E-04, -1.334127E-05})}};
    }
};

}  // namespace framework
