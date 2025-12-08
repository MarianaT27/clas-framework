#pragma once

#include <Framework/IService.hpp>
#include <Framework/StatusCode.hpp>
#include <CLAS/Target.hpp>

#include <array>
#include <string>
#include <unordered_map>

namespace Framework {

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
 * This service provides target and polarity-dependent sampling fraction
 * parameterizations for electron identification. It is implemented as a
 * singleton-like service that can be shared across algorithms.
 *
 * The sampling fraction is parameterized as:
 *   SF = μ(p) ± n·σ(p)
 *
 * where μ and σ are third-order polynomials in momentum p.
 *
 * Usage:
 * @code
 * auto sf_svc = service<Framework::SamplingFractionService>("SamplingFractionService");
 *
 * double mu = sf_svc->evaluate_mu("LD2", "OB", sector, momentum);
 * double sigma = sf_svc->evaluate_sigma("LD2", "OB", sector, momentum);
 *
 * // Or use convenience methods
 * bool passes = sf_svc->passes_cut(SF, momentum, sector, CLAS::Target::LD2, CLAS::Polarity::Outbending);
 * @endcode
 */
class SamplingFractionService : public Service {
public:
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
     * @brief Get parameters for a specific target, polarity, and sector
     */
    [[nodiscard]] auto get_parameters(const std::string& target, const std::string& polarity, int sector) const
        -> const SectorSFParameters& {
        return m_data.at(target + "_" + polarity)[sector - 1];
    }

    /**
     * @brief Evaluate μ for given target, polarity, sector, and momentum
     */
    [[nodiscard]] auto evaluate_mu(const std::string& target, const std::string& polarity, int sector, double p) const
        -> double {
        return get_parameters(target, polarity, sector).mu.evaluate(p);
    }

    /**
     * @brief Evaluate σ for given target, polarity, sector, and momentum
     */
    [[nodiscard]] auto evaluate_sigma(const std::string& target, const std::string& polarity, int sector, double p) const
        -> double {
        return get_parameters(target, polarity, sector).sigma.evaluate(p);
    }

    /**
     * @brief Evaluate μ + n·σ (upper cut)
     */
    [[nodiscard]] auto evaluate_plus_cut(const std::string& target, const std::string& polarity, int sector, double p, double n) const
        -> double {
        const auto& params = get_parameters(target, polarity, sector);
        return params.mu.evaluate(p) + n * params.sigma.evaluate(p);
    }

    /**
     * @brief Evaluate μ - n·σ (lower cut)
     */
    [[nodiscard]] auto evaluate_minus_cut(const std::string& target, const std::string& polarity, int sector, double p, double n) const
        -> double {
        const auto& params = get_parameters(target, polarity, sector);
        return params.mu.evaluate(p) - n * params.sigma.evaluate(p);
    }

    // ============================================================================
    // Convenience Methods with CLAS::Target/Polarity
    // ============================================================================

    /**
     * @brief Check if sampling fraction passes the cut
     */
    [[nodiscard]] auto passes_cut(double SF, double p, int sector, CLAS::Target target, CLAS::Polarity polarity) const
        -> bool {
        return passes_cut(SF, p, sector, target, polarity, m_n_sigma);
    }

    /**
     * @brief Check if sampling fraction passes the cut with custom n_sigma
     */
    [[nodiscard]] auto passes_cut(double SF, double p, int sector, CLAS::Target target, CLAS::Polarity polarity, double n_sigma) const
        -> bool {
        if (sector < 1 || sector > NUM_SECTORS) return false;

        std::string target_str = CLAS::to_string(target);
        std::string polarity_str = (polarity == CLAS::Polarity::Outbending) ? "OB" : "IB";

        double cut_low = evaluate_minus_cut(target_str, polarity_str, sector, p, n_sigma);
        double cut_high = evaluate_plus_cut(target_str, polarity_str, sector, p, n_sigma);

        return cut_low < SF && SF < cut_high;
    }

    /**
     * @brief Get μ value using Core types
     */
    [[nodiscard]] auto get_mu(double p, int sector, CLAS::Target target, CLAS::Polarity polarity) const
        -> double {
        std::string target_str = CLAS::to_string(target);
        std::string polarity_str = (polarity == CLAS::Polarity::Outbending) ? "OB" : "IB";
        return evaluate_mu(target_str, polarity_str, sector, p);
    }

    /**
     * @brief Get σ value using Core types
     */
    [[nodiscard]] auto get_sigma(double p, int sector, CLAS::Target target, CLAS::Polarity polarity) const
        -> double {
        std::string target_str = CLAS::to_string(target);
        std::string polarity_str = (polarity == CLAS::Polarity::Outbending) ? "OB" : "IB";
        return evaluate_sigma(target_str, polarity_str, sector, p);
    }

private:
    std::unordered_map<std::string, std::array<SectorSFParameters, NUM_SECTORS>> m_data;
    double m_n_sigma = DEFAULT_N_SIGMA;

    void initialize_data() {
        // LD2 Outbending
        m_data["LD2_OB"] = {{
            SectorSFParameters(
                {0.23888096534759243, 0.003192082668648829, -0.0005843795153379318, 1.3664308231698308e-05},
                {0.025200794392991534, -0.00470765393875185, 0.000528134582986412, -2.21383622877987e-05}),
            SectorSFParameters(
                {0.23745547449150275, 0.0026165843015185705, -0.0002477280859333332, -1.082851195983763e-05},
                {0.02450258833202397, -0.004276831058281573, 0.00045955686503876004, -1.70298672651076e-05}),
            SectorSFParameters(
                {0.2377345478877124, 0.005179762426145616, -0.0007976897320080561, 1.8901061223088483e-05},
                {0.026098904108214718, -0.005331842900972835, 0.0006342003963986534, -2.716277937997175e-05}),
            SectorSFParameters(
                {0.23803105386902598, 0.0036417511392235393, -0.000427038732735186, 3.649420307358638e-07},
                {0.025152734759029956, -0.004304430572196834, 0.00045423767914342744, -1.830031649600036e-05}),
            SectorSFParameters(
                {0.2366387660096871, 0.0020793555615445354, -9.299134288306175e-05, -1.3591045239626094e-05},
                {0.0245651100199992, -0.004744762148443335, 0.0005513690936278644, -2.3384525935234174e-05}),
            SectorSFParameters(
                {0.23350263669467658, 0.0052655266175207285, -0.0007058996583564587, 1.1653514153603771e-05},
                {0.025750062104874256, -0.004929916311342956, 0.0005695853682280201, -2.4197396235041972e-05})
        }};

        // LD2 Inbending
        m_data["LD2_IB"] = {{
            SectorSFParameters(
                {0.24348099926252276, 0.0004315025946443613, -6.597422149273941e-05, -2.335291802477954e-05},
                {0.027584638609700254, -0.005163732682539585, 0.0006091507731397759, -2.368432110379916e-05}),
            SectorSFParameters(
                {0.24962867248792034, -0.003391556296034612, 0.0014819953430239244, -0.00017723269381797382},
                {0.028507483755929732, -0.006079484475409221, 0.00076961186317557, -1.4483516136052e-05}),
            SectorSFParameters(
                {0.2572074246356512, -0.00841688284783703, 0.0027028641629357245, -0.0002777174712614641},
                {0.03186691361808035, -0.008874292844933292, 0.0013689495678676001, -5.3825493810764944e-05}),
            SectorSFParameters(
                {0.2487072195486556, -0.0014131395777636146, 0.0010600224654544691, -0.0001409716927049328},
                {0.030712888570724302, -0.0059101101616285505, 0.0005545808233686116, -1.9961080943367665e-06}),
            SectorSFParameters(
                {0.2532897257243984, -0.0064482139706124614, 0.0022686392185830174, -0.00022266924472217204},
                {0.025420490121785013, -0.00317332786914108, -8.578569079879491e-05, 4.426980177285514e-05}),
            SectorSFParameters(
                {0.2519960895158266, -0.0051106896226398805, 0.0017660915596951106, -0.00019192952965443497},
                {0.02985494818187786, -0.006586309213029568, 0.0007453945504715554, -1.1769323944661249e-05})
        }};

        // CxC Outbending
        m_data["CxC_OB"] = {{
            SectorSFParameters(
                {0.23861468749241596, 0.003430867557708171, -0.0006200007386259007, 1.551211518888557e-05},
                {0.024356815119914544, -0.004277841641313608, 0.00046235397537642317, -1.8943448629767313e-05}),
            SectorSFParameters(
                {0.23746553265370046, 0.003023372005294365, -0.00031869279809046, -6.770038408165503e-06},
                {0.023521607045916288, -0.003801067190327984, 0.0003941585251879201, -1.4115352750558311e-05}),
            SectorSFParameters(
                {0.2399330431057671, 0.004367094227908059, -0.0006921907508747271, 1.454278916100893e-05},
                {0.025490909116408403, -0.005065721919683289, 0.0005995425483128538, -2.5624112796951298e-05}),
            SectorSFParameters(
                {0.23905035019136944, 0.003396762006632739, -0.0004041352071480395, 2.6213971860504768e-08},
                {0.024202676210469395, -0.003818757757880008, 0.0003788844775111284, -1.4567754903035795e-05}),
            SectorSFParameters(
                {0.2382052448390636, 0.0016202865107867368, -3.192507101588507e-05, -1.6198516915431723e-05},
                {0.023794412019131385, -0.004300673768458812, 0.0004816005565311549, -1.990334121402402e-05}),
            SectorSFParameters(
                {0.23281538297832863, 0.006117509581883395, -0.0008709471756641501, 2.087630490786824e-05},
                {0.025161420427095703, -0.004648182980385569, 0.0005308213424029133, -2.2484349716497945e-05})
        }};

        // CxC Inbending
        m_data["CxC_IB"] = {{
            SectorSFParameters(
                {0.23380874281639547, 0.005468039761357135, -0.0009897006149941609, 2.9321298589983806e-05},
                {0.01673823193474205, 0.0013133946328399112, -0.0005894468123160733, 4.582142411000439e-05}),
            SectorSFParameters(
                {0.24839375571390596, -0.002955732652790467, 0.001424644738034184, -0.00017724094728567596},
                {0.03167731078843818, -0.008667906288995348, 0.0013885401747394848, -5.618585175062376e-05}),
            SectorSFParameters(
                {0.25301032856442063, -0.006858632901205996, 0.0023545006163521575, -0.0002540244544927149},
                {0.03040028257328834, -0.007361876491228678, 0.0009469523159350211, -2.066980650387891e-05}),
            SectorSFParameters(
                {0.2473582060913954, -0.001766999147612072, 0.0010479366722689165, -0.00013401036022018883},
                {0.030655891169713536, -0.006849600054159684, 0.0008947585742311191, -3.075882272057716e-05}),
            SectorSFParameters(
                {0.25358329646020983, -0.00788448219675817, 0.00243118419962511, -0.00022278148801931606},
                {0.026221285468885743, -0.004098067334899864, 0.00019827068373344119, 1.838024764179674e-05}),
            SectorSFParameters(
                {0.2495357588720735, -0.003956816605520869, 0.0015126175205506444, -0.00017504938750812524},
                {0.0290126187687071, -0.006133481134367889, 0.0006573628992165562, -5.595565574544266e-06})
        }};

        // CuSn Outbending
        m_data["CuSn_OB"] = {{
            SectorSFParameters(
                {0.250098273134676, -0.003913731425312631, 0.0007642115702407434, -7.262343698893812e-05},
                {0.023302885565687135, -0.0027082087458571947, 0.00018090384859018786, 2.7946171203870004e-07}),
            SectorSFParameters(
                {0.2491004750941496, -0.003740746382139163, 0.001598446723410039, -0.000189371042629573},
                {0.032075890093076834, -0.008812708852219096, 0.0014079803109118276, -5.6450441230883824e-05}),
            SectorSFParameters(
                {0.25126737437620184, -0.005939357616789668, 0.0021605289396475266, -0.000242392954733982},
                {0.030103705568131002, -0.0067988053925147895, 0.0007712050172052243, -5.481360098178394e-06}),
            SectorSFParameters(
                {0.24939319008180794, -0.0030097869037308427, 0.0012473344200845322, -0.00014371864483084522},
                {0.026140803321118438, -0.0032120410544449808, 4.4938856494187554e-05, 2.8884822846169236e-05}),
            SectorSFParameters(
                {0.2509682471608173, -0.006602820848286794, 0.002217697546195483, -0.00021327564003420786},
                {0.025275985784346512, -0.0030257939207007215, -0.00011971320742326094, 4.5974669064025907e-05}),
            SectorSFParameters(
                {0.2478767805620501, -0.0029177133657701635, 0.0012956043826830377, -0.00016150809017220405},
                {0.03147938848498606, -0.007553784007642037, 0.0009112810088149259, -1.8807370419896238e-05})
        }};

        // CuSn Inbending (same as OB for now)
        m_data["CuSn_IB"] = m_data["CuSn_OB"];

        // Cu and Sn use CuSn parameters
        m_data["Cu_OB"] = m_data["CuSn_OB"];
        m_data["Sn_OB"] = m_data["CuSn_OB"];
        m_data["Cu_IB"] = m_data["CuSn_IB"];
        m_data["Sn_IB"] = m_data["CuSn_IB"];
    }
};

}  // namespace Framework
