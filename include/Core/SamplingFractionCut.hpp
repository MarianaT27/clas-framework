#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>

namespace Core {

// Structure to hold polynomial coefficients
struct PolynomialCoefficients {
    double a, b, c, d;

    PolynomialCoefficients(double a = 0.0, double b = 0.0, double c = 0.0, double d = 0.0)
        : a(a), b(b), c(c), d(d) {}

    // Evaluate polynomial: a + b*x + c*x^2 + d*x^3
    double evaluate(double x) const {
        return a + b * x + c * x * x + d * x * x * x;
    }
};

// Structure for a single sector's parameters
struct SectorParameters {
    PolynomialCoefficients mu;
    PolynomialCoefficients sigma;

    SectorParameters() = default;

    SectorParameters(const PolynomialCoefficients& mu_coeffs,
                     const PolynomialCoefficients& sigma_coeffs)
        : mu(mu_coeffs), sigma(sigma_coeffs) {}
};

// Main material coefficients structure
class SamplingFractionCut {
   public:
    static constexpr int NUM_SECTORS = 6;

   private:
    std::unordered_map<std::string, std::array<SectorParameters, NUM_SECTORS>> data;

    void initialize_data() {
        data["LD2_OB"] = {
            // Sector 1
            SectorParameters(
                {0.23888096534759243, 0.003192082668648829, -0.0005843795153379318, 1.3664308231698308e-05},
                {0.025200794392991534, -0.00470765393875185, 0.000528134582986412, -2.21383622877987e-05}),
            // Sector 2
            SectorParameters(
                {0.23745547449150275, 0.0026165843015185705, -0.0002477280859333332, -1.082851195983763e-05},
                {0.02450258833202397, -0.004276831058281573, 0.00045955686503876004, -1.70298672651076e-05}),
            // Sector 3
            SectorParameters(
                {0.2377345478877124, 0.005179762426145616, -0.0007976897320080561, 1.8901061223088483e-05},
                {0.026098904108214718, -0.005331842900972835, 0.0006342003963986534, -2.716277937997175e-05}),
            // Sector 4
            SectorParameters(
                {0.23803105386902598, 0.0036417511392235393, -0.000427038732735186, 3.649420307358638e-07},
                {0.025152734759029956, -0.004304430572196834, 0.00045423767914342744, -1.830031649600036e-05}),
            // Sector 5
            SectorParameters(
                {0.2366387660096871, 0.0020793555615445354, -9.299134288306175e-05, -1.3591045239626094e-05},
                {0.0245651100199992, -0.004744762148443335, 0.0005513690936278644, -2.3384525935234174e-05}),
            // Sector 6
            SectorParameters(
                {0.23350263669467658, 0.0052655266175207285, -0.0007058996583564587, 1.1653514153603771e-05},
                {0.025750062104874256, -0.004929916311342956, 0.0005695853682280201, -2.4197396235041972e-05})};

        data["LD2_IB"] = {
            // Sector 1
            SectorParameters(
                {0.24348099926252276, 0.0004315025946443613, -6.597422149273941e-05, -2.335291802477954e-05},
                {0.027584638609700254, -0.005163732682539585, 0.0006091507731397759, -2.368432110379916e-05}),
            // Sector 2
            SectorParameters(
                {0.24962867248792034, -0.003391556296034612, 0.0014819953430239244, -0.00017723269381797382},
                {0.028507483755929732, -0.006079484475409221, 0.00076961186317557, -1.4483516136052e-05}),
            // Sector 3
            SectorParameters(
                {0.2572074246356512, -0.00841688284783703, 0.0027028641629357245, -0.0002777174712614641},
                {0.03186691361808035, -0.008874292844933292, 0.0013689495678676001, -5.3825493810764944e-05}),
            // Sector 4
            SectorParameters(
                {0.2487072195486556, -0.0014131395777636146, 0.0010600224654544691, -0.0001409716927049328},
                {0.030712888570724302, -0.0059101101616285505, 0.0005545808233686116, -1.9961080943367665e-06}),
            // Sector 5
            SectorParameters(
                {0.2532897257243984, -0.0064482139706124614, 0.0022686392185830174, -0.00022266924472217204},
                {0.025420490121785013, -0.00317332786914108, -8.578569079879491e-05, 4.426980177285514e-05}),
            // Sector 6
            SectorParameters(
                {0.2519960895158266, -0.0051106896226398805, 0.0017660915596951106, -0.00019192952965443497},
                {0.02985494818187786, -0.006586309213029568, 0.0007453945504715554, -1.1769323944661249e-05})};

        data["CxC_OB"] = {
            // Sector 1
            SectorParameters(
                {0.23861468749241596, 0.003430867557708171, -0.0006200007386259007, 1.551211518888557e-05},
                {0.024356815119914544, -0.004277841641313608, 0.00046235397537642317, -1.8943448629767313e-05}),
            // Sector 2
            SectorParameters(
                {0.23746553265370046, 0.003023372005294365, -0.00031869279809046, -6.770038408165503e-06},
                {0.023521607045916288, -0.003801067190327984, 0.0003941585251879201, -1.4115352750558311e-05}),
            // Sector 3
            SectorParameters(
                {0.2399330431057671, 0.004367094227908059, -0.0006921907508747271, 1.454278916100893e-05},
                {0.025490909116408403, -0.005065721919683289, 0.0005995425483128538, -2.5624112796951298e-05}),
            // Sector 4
            SectorParameters(
                {0.23905035019136944, 0.003396762006632739, -0.0004041352071480395, 2.6213971860504768e-08},
                {0.024202676210469395, -0.003818757757880008, 0.0003788844775111284, -1.4567754903035795e-05}),
            // Sector 5
            SectorParameters(
                {0.2382052448390636, 0.0016202865107867368, -3.192507101588507e-05, -1.6198516915431723e-05},
                {0.023794412019131385, -0.004300673768458812, 0.0004816005565311549, -1.990334121402402e-05}),
            // Sector 6
            SectorParameters(
                {0.23281538297832863, 0.006117509581883395, -0.0008709471756641501, 2.087630490786824e-05},
                {0.025161420427095703, -0.004648182980385569, 0.0005308213424029133, -2.2484349716497945e-05})};

        data["CxC_IB"] = {
            // Sector 1
            SectorParameters(
                {0.23380874281639547, 0.005468039761357135, -0.0009897006149941609, 2.9321298589983806e-05},
                {0.01673823193474205, 0.0013133946328399112, -0.0005894468123160733, 4.582142411000439e-05}),
            // Sector 2
            SectorParameters(
                {0.24839375571390596, -0.002955732652790467, 0.001424644738034184, -0.00017724094728567596},
                {0.03167731078843818, -0.008667906288995348, 0.0013885401747394848, -5.618585175062376e-05}),
            // Sector 3
            SectorParameters(
                {0.25301032856442063, -0.006858632901205996, 0.0023545006163521575, -0.0002540244544927149},
                {0.03040028257328834, -0.007361876491228678, 0.0009469523159350211, -2.066980650387891e-05}),
            // Sector 4
            SectorParameters(
                {0.2473582060913954, -0.001766999147612072, 0.0010479366722689165, -0.00013401036022018883},
                {0.030655891169713536, -0.006849600054159684, 0.0008947585742311191, -3.075882272057716e-05}),
            // Sector 5
            SectorParameters(
                {0.25358329646020983, -0.00788448219675817, 0.00243118419962511, -0.00022278148801931606},
                {0.026221285468885743, -0.004098067334899864, 0.00019827068373344119, 1.838024764179674e-05}),
            // Sector 6
            SectorParameters(
                {0.2495357588720735, -0.003956816605520869, 0.0015126175205506444, -0.00017504938750812524},
                {0.0290126187687071, -0.006133481134367889, 0.0006573628992165562, -5.595565574544266e-06})};

        data["CuSn_OB"] = {
            // Sector 1
            SectorParameters(
                {0.250098273134676, -0.003913731425312631, 0.0007642115702407434, -7.262343698893812e-05},
                {0.023302885565687135, -0.0027082087458571947, 0.00018090384859018786, 2.7946171203870004e-07}),
            // Sector 2
            SectorParameters(
                {0.2491004750941496, -0.003740746382139163, 0.001598446723410039, -0.000189371042629573},
                {0.032075890093076834, -0.008812708852219096, 0.0014079803109118276, -5.6450441230883824e-05}),
            // Sector 3
            SectorParameters(
                {0.25126737437620184, -0.005939357616789668, 0.0021605289396475266, -0.000242392954733982},
                {0.030103705568131002, -0.0067988053925147895, 0.0007712050172052243, -5.481360098178394e-06}),
            // Sector 4
            SectorParameters(
                {0.24939319008180794, -0.0030097869037308427, 0.0012473344200845322, -0.00014371864483084522},
                {0.026140803321118438, -0.0032120410544449808, 4.4938856494187554e-05, 2.8884822846169236e-05}),
            // Sector 5
            SectorParameters(
                {0.2509682471608173, -0.006602820848286794, 0.002217697546195483, -0.00021327564003420786},
                {0.025275985784346512, -0.0030257939207007215, -0.00011971320742326094, 4.5974669064025907e-05}),
            // Sector 6
            SectorParameters(
                {0.2478767805620501, -0.0029177133657701635, 0.0012956043826830377, -0.00016150809017220405},
                {0.03147938848498606, -0.007553784007642037, 0.0009112810088149259, -1.8807370419896238e-05})};

        data["CuSn_IB"] = {
            // Sector 1
            SectorParameters(
                {0.250098273134676, -0.003913731425312631, 0.0007642115702407434, -7.262343698893812e-05},
                {0.023302885565687135, -0.0027082087458571947, 0.00018090384859018786, 2.7946171203870004e-07}),
            // Sector 2
            SectorParameters(
                {0.2491004750941496, -0.003740746382139163, 0.001598446723410039, -0.000189371042629573},
                {0.032075890093076834, -0.008812708852219096, 0.0014079803109118276, -5.6450441230883824e-05}),
            // Sector 3
            SectorParameters(
                {0.25126737437620184, -0.005939357616789668, 0.0021605289396475266, -0.000242392954733982},
                {0.030103705568131002, -0.0067988053925147895, 0.0007712050172052243, -5.481360098178394e-06}),
            // Sector 4
            SectorParameters(
                {0.24939319008180794, -0.0030097869037308427, 0.0012473344200845322, -0.00014371864483084522},
                {0.026140803321118438, -0.0032120410544449808, 4.4938856494187554e-05, 2.8884822846169236e-05}),
            // Sector 5
            SectorParameters(
                {0.2509682471608173, -0.006602820848286794, 0.002217697546195483, -0.00021327564003420786},
                {0.025275985784346512, -0.0030257939207007215, -0.00011971320742326094, 4.5974669064025907e-05}),
            // Sector 6
            SectorParameters(
                {0.2478767805620501, -0.0029177133657701635, 0.0012956043826830377, -0.00016150809017220405},
                {0.03147938848498606, -0.007553784007642037, 0.0009112810088149259, -1.8807370419896238e-05})};

        data["Cu_OB"] = data["CuSn_OB"];
        data["Sn_OB"] = data["CuSn_OB"];
        data["Cu_IB"] = data["CuSn_IB"];
        data["Sn_IB"] = data["CuSn_IB"];
    }

   public:
    SamplingFractionCut() {
        initialize_data();
    }

    // Get parameters for a specific target and sector
    const SectorParameters& get_parameters(const std::string& target, const std::string& polarity, int sector) const {
        if (sector < 1 || sector > NUM_SECTORS) std::cout << "Sector must be between 1 and " + std::to_string(NUM_SECTORS) << std::endl;
        return data.at(target + "_" + polarity)[sector - 1];
    }

    /** Evaluate mu for given target, sector, and x value
        * @param target The target material ("LD2", "CxC", "CuSn")
        * @param polarity The torus polarity ("IB" or "OB")
        * @param sector The sector number (1 to 6)
        * @param p The momentum value
        * @return The value of μ for the given parameters
     */
    double evaluate_mu(const std::string& target, const std::string& polarity, int sector, double p) const { return get_parameters(target, polarity, sector).mu.evaluate(p); }

    /** Evaluate sigma for given target, sector, and x value
        * @param target The target material ("LD2", "CxC", "CuSn")
        * @param polarity The torus polarity ("IB" or "OB")
        * @param sector The sector number (1 to 6)
        * @param p The momentum value
        * @return The value of σ for the given parameters
     */
    double evaluate_sigma(const std::string& target, const std::string& polarity, int sector, double p) const { return get_parameters(target, polarity, sector).sigma.evaluate(p); }

    /** Return μ + nσ
        * @param target The target material ("LD2", "CxC", "CuSn")
        * @param polarity The torus polarity ("IB" or "OB")
        * @param sector The sector number (1 to 6)
        * @param p The momentum value
        * @param n The number of standard deviations to add (e.g., 3 for 3σ)
        * @return The value of μ + nσ for the given parameters
     */
    double evaluate_plus_cut(const std::string& target, const std::string& polarity, int sector, double p, double n) const { return get_parameters(target, polarity, sector).mu.evaluate(p) + n * get_parameters(target, polarity, sector).sigma.evaluate(p); }

    /** Return μ - nσ
        * @param target The target material ("LD2", "CxC", "CuSn")
        * @param polarity The torus polarity ("IB" or "OB")
        * @param sector The sector number (1 to 6)
        * @param p The momentum value
        * @param n The number of standard deviations to subtract (e.g., 3 for 3σ)
        * @return The value of μ - nσ for the given parameters
     */
    double evaluate_minus_cut(const std::string& target, const std::string& polarity, int sector, double p, double n) const { return get_parameters(target, polarity, sector).mu.evaluate(p) - n * get_parameters(target, polarity, sector).sigma.evaluate(p); }
};

}  // namespace Core
