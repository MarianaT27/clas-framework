#pragma once

#include <Math/Vector4D.h>
#include <cmath>
#include <optional>
#include <vector>
#include <clas12/Constants.hpp>
#include <clas12/PIDs.hpp>
#include <clas12/bankdefs/DECAYS_Particle.hpp>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Cutflow.hpp>
#include <clas12/framework/core/Decisions.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/NTupleStruct.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace algorithms {

using framework::Log;

constexpr int LAMBDA_PID = 3122;
constexpr double LAMBDA_MASS_PDG = 1.115683;

struct DecaysLambda {
    int idx = -1;
    int pid = 0;
    double mass = 0.0;
    double umass = 0.0;
    double px = 0.0, py = 0.0, pz = 0.0;
    double vx = 0.0, vy = 0.0, vz = 0.0;
    double ovx = 0.0, ovy = 0.0, ovz = 0.0;
    double e = 0.0;
    double emc = 0.0;
    double erec = 0.0;
    int charge = 0;
    int ndau = 0;
    int dau1idx = 0, dau2idx = 0, dau3idx = 0;
    int det = 0;
    double r = 0.0;

    auto momentum() const -> double { return std::sqrt(px * px + py * py + pz * pz); }
    auto theta() const -> double { return std::acos(pz / momentum()); }
    auto phi() const -> double { return std::atan2(py, px); }

    auto decay_length() const -> double {
        double dx = vx - ovx;
        double dy = vy - ovy;
        double dz = vz - ovz;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    auto vertex_detach_z() const -> double { return vz - ovz; }

    auto vertex_detach_xy() const -> double {
        double dx = vx - ovx;
        double dy = vy - ovy;
        return std::sqrt(dx * dx + dy * dy);
    }
};

struct DecaysDaughter {
    int idx = -1;
    int pid = 0;
    double mass = 0.0;
    double px = 0.0, py = 0.0, pz = 0.0;
    double vx = 0.0, vy = 0.0, vz = 0.0;
    double upx = 0.0, upy = 0.0, upz = 0.0;
    int charge = 0;
    int det = 0;

    auto momentum() const -> double { return std::sqrt(px * px + py * py + pz * pz); }
    auto theta() const -> double { return std::acos(pz / momentum()); }
    auto phi() const -> double { return std::atan2(py, px); }
};

/// Configuration for DecaysLambdaAlg.
struct DecaysLambdaConfig {
    // Reserved for future tuning knobs that aren't user-tunable Properties.

    double mass_window = 0.1;
    double min_momentum = 0.0;
    double min_decay_length = 0.0;
    double min_vertex_detach = -1000.0;
    double max_vertex_detach = 0.0;
    bool enable_mass_window_cut = true;
    bool enable_momentum_cut = true;
    bool enable_decay_length_cut = true;
    bool enable_vertex_detach_cut = true;

    static constexpr auto config_fields() {
        using T = DecaysLambdaConfig;
        return std::make_tuple(
            framework::field("mass_window", &T::mass_window),
            framework::field("min_momentum", &T::min_momentum),
            framework::field("min_decay_length", &T::min_decay_length),
            framework::field("min_vertex_detach", &T::min_vertex_detach),
            framework::field("max_vertex_detach", &T::max_vertex_detach),
            framework::field("enable_mass_window_cut", &T::enable_mass_window_cut),
            framework::field("enable_momentum_cut", &T::enable_momentum_cut),
            framework::field("enable_decay_length_cut", &T::enable_decay_length_cut),
            framework::field("enable_vertex_detach_cut", &T::enable_vertex_detach_cut));
    }
};

/**
 * @brief Algorithm for Lambda analysis using DECAYS::Particle bank
 */
class DecaysLambdaAlg : public framework::Algorithm {
   public:
    using Config = DecaysLambdaConfig;
    using StatusCode = framework::StatusCode;

    explicit DecaysLambdaAlg(std::string name = "DecaysLambdaAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("DECAYS Lambda Analysis") {
        required_services(m_hist, m_ntuple);
        declare_banks("DECAYS::Particle");
        declare_config(m_config);
        declare_histograms(h_n_lambdas, h_mass_nocuts, h_mass_selected, h_umass, h_momentum, h_theta, h_phi, h_decay_length, h_decay_vz, h_origin_vz, h_r, h_ndau, h_proton_p, h_pion_p, h_mass_vs_p, h_decay_vz_vs_origin_vz, h_vertex_detach, h_vertex_detach_precut, h_vertex_detach_xy);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing DECAYS Lambda analysis algorithm");
        Log::info(name(), "  Mass window: {} GeV", m_config.mass_window);
        Log::info(name(), "  Min momentum: {} GeV", m_config.min_momentum);
        Log::info(name(), "  Min decay length: {} cm", m_config.min_decay_length);
        Log::info(name(), "  Min vertex detach: {} cm", m_config.min_vertex_detach);
        Log::info(name(), "  Max vertex detach: {} cm", m_config.max_vertex_detach);

        m_ntuple_writer.init(m_ntuple.get(), "decays_lambda");
        Log::info(name(), "N-tuple branches registered for DECAYS Lambda analysis");

        m_cutflow.add_cut("All Events");
        m_cutflow.add_cut("Has DECAYS Bank");
        m_cutflow.add_cut("Has Lambda");
        m_cutflow.add_cut("Mass Window");
        m_cutflow.add_cut("Momentum Cut");
        m_cutflow.add_cut("Decay Length Cut");
        m_cutflow.add_cut("Vertex Detach Cut");

        Log::info(name(), "Initialized with {} cuts in cutflow", m_cutflow.num_cuts());

        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        bankdefs::DECAYS_Particle decays;
        if (!decays.bind(event.get_bank("DECAYS::Particle"))) return StatusCode::FILTERED();

        const int nrows = decays.rows();
        if (nrows == 0) return StatusCode::FILTERED();
        m_cutflow.pass("Has DECAYS Bank");

        std::vector<DecaysLambda> lambdas;
        std::vector<DecaysDaughter> all_daughters;

        for (int i = 0; i < nrows; ++i) {
            const int pid = decays.pid(i);
            if (pid == LAMBDA_PID) {
                DecaysLambda lambda;
                lambda.idx = decays.idx(i);
                lambda.pid = pid;
                lambda.mass = decays.mass(i);
                lambda.umass = decays.umass(i);
                lambda.px = decays.px(i);
                lambda.py = decays.py(i);
                lambda.pz = decays.pz(i);
                lambda.vx = decays.vx(i);
                lambda.vy = decays.vy(i);
                lambda.vz = decays.vz(i);
                lambda.ovx = decays.ovx(i);
                lambda.ovy = decays.ovy(i);
                lambda.ovz = decays.ovz(i);
                lambda.e = decays.e(i);
                lambda.emc = decays.emc(i);
                lambda.erec = decays.erec(i);
                lambda.charge = decays.charge(i);
                lambda.ndau = decays.ndau(i);
                lambda.dau1idx = decays.dau1idx(i);
                lambda.dau2idx = decays.dau2idx(i);
                lambda.dau3idx = decays.dau3idx(i);
                lambda.det = decays.det(i);
                lambda.r = decays.r(i);
                lambdas.push_back(lambda);
            } else {
                DecaysDaughter dau;
                dau.idx = decays.idx(i);
                dau.pid = pid;
                dau.mass = decays.mass(i);
                dau.px = decays.px(i);
                dau.py = decays.py(i);
                dau.pz = decays.pz(i);
                dau.vx = decays.vx(i);
                dau.vy = decays.vy(i);
                dau.vz = decays.vz(i);
                dau.upx = decays.upx(i);
                dau.upy = decays.upy(i);
                dau.upz = decays.upz(i);
                dau.charge = decays.charge(i);
                dau.det = decays.det(i);
                all_daughters.push_back(dau);
            }
        }

        if (lambdas.empty()) return StatusCode::FILTERED();
        m_cutflow.pass("Has Lambda");

        h_n_lambdas.fill(lambdas.size());
        for (const auto& lambda : lambdas) {
            h_mass_nocuts.fill(lambda.mass);
        }

        std::vector<DecaysLambda> selected_lambdas;
        bool passed_mass = false;
        bool passed_momentum = false;
        bool passed_decay_length = false;
        bool passed_vertex_detach = false;

        for (const auto& lambda : lambdas) {
            h_vertex_detach_precut.fill(lambda.vertex_detach_z());
            if (m_config.enable_mass_window_cut && std::abs(lambda.mass - LAMBDA_MASS_PDG) > m_config.mass_window) continue;
            passed_mass = true;

            if (m_config.enable_momentum_cut && lambda.momentum() < m_config.min_momentum) continue;
            passed_momentum = true;

            if (m_config.enable_decay_length_cut && m_config.min_decay_length > 0 && lambda.decay_length() < m_config.min_decay_length) continue;
            passed_decay_length = true;

            if (m_config.enable_vertex_detach_cut && m_config.min_vertex_detach > -999) {
                double detach = lambda.vertex_detach_z();
                if (detach < m_config.min_vertex_detach) continue;
                if (m_config.max_vertex_detach > 0 && detach > m_config.max_vertex_detach) continue;
            }
            passed_vertex_detach = true;

            selected_lambdas.push_back(lambda);
        }

        if (!passed_mass) return StatusCode::FILTERED();
        m_cutflow.pass("Mass Window");
        if (!passed_momentum) return StatusCode::FILTERED();
        m_cutflow.pass("Momentum Cut");
        if (!passed_decay_length) return StatusCode::FILTERED();
        m_cutflow.pass("Decay Length Cut");
        if (!passed_vertex_detach) return StatusCode::FILTERED();
        m_cutflow.pass("Vertex Detach Cut");

        if (selected_lambdas.empty()) return StatusCode::FILTERED();

        event.set_decision_typed(framework::Decisions::HasDecaysLambda, true);

        auto best_it = std::min_element(selected_lambdas.begin(), selected_lambdas.end(), [](const DecaysLambda& a, const DecaysLambda& b) { return std::abs(a.mass - LAMBDA_MASS_PDG) < std::abs(b.mass - LAMBDA_MASS_PDG); });
        event.store_typed(framework::Keys::DecaysLambdaMass, best_it->mass);
        event.store_typed(framework::Keys::DecaysLambdaMomentum, best_it->momentum());

        for (const auto& lambda : selected_lambdas) fill_lambda_histograms(lambda, all_daughters);

        if (m_ntuple) fill_ntuple(selected_lambdas, all_daughters);

        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(name(), "Finalizing DECAYS Lambda analysis algorithm");
        if (m_ntuple) Log::info(name(), "N-tuple has {} entries", m_ntuple->get_entries("decays_lambda"));
        m_cutflow.print();
        return StatusCode::SUCCESS();
    }

   private:
    Config m_config;

framework::Cutflow m_cutflow;

    framework::ServiceRef<framework::HistogramService> m_hist;
    framework::ServiceRef<framework::NTupleService> m_ntuple;

    framework::Hist1<> h_n_lambdas{"/DecaysLambda/n_lambdas", "Number of #Lambda per event;N_{#Lambda};Events", 0, 10, 10};
    framework::Hist1<> h_mass_nocuts{"/DecaysLambda/mass_nocuts", "#Lambda mass (no cuts);M [GeV];Events", 1.0, 1.3, 200};
    framework::Hist1<> h_mass_selected{"/DecaysLambda/mass_selected", "#Lambda mass (selected);M [GeV];Events", 1.0, 1.3, 200};
    framework::Hist1<> h_umass{"/DecaysLambda/umass", "#Lambda unmeasured mass;M [GeV];Events", 1.0, 1.3, 200};
    framework::Hist1<> h_momentum{"/DecaysLambda/momentum", "#Lambda momentum;p [GeV];Events", 0, 10, 100};
    framework::Hist1<> h_theta{"/DecaysLambda/theta", "#Lambda #theta;#theta [deg];Events", 0, 90, 90};
    framework::Hist1<> h_phi{"/DecaysLambda/phi", "#Lambda #phi;#phi [deg];Events", -180, 180, 180};
    framework::Hist1<> h_decay_length{"/DecaysLambda/decay_length", "#Lambda decay length;L [cm];Events", 0, 50, 100};
    framework::Hist1<> h_decay_vz{"/DecaysLambda/decay_vz", "#Lambda decay vertex z;v_{z} [cm];Events", -30, 30, 200};
    framework::Hist1<> h_origin_vz{"/DecaysLambda/origin_vz", "#Lambda origin vertex z;ov_{z} [cm];Events", -30, 30, 200};
    framework::Hist1<> h_r{"/DecaysLambda/r", "#Lambda decay radius;r [cm];Events", 0, 20, 100};
    framework::Hist1<> h_ndau{"/DecaysLambda/ndau", "Number of daughters;N_{dau};Events", 0, 5, 5};
    framework::Hist1<> h_proton_p{"/DecaysLambda/proton_p", "Proton momentum;p [GeV];Events", 0, 5, 100};
    framework::Hist1<> h_pion_p{"/DecaysLambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 0, 3, 100};
    framework::Hist2<> h_mass_vs_p{"/DecaysLambda/mass_vs_p", "#Lambda mass vs momentum;p [GeV];M [GeV]", 0, 10, 100, 1.0, 1.3, 100};
    framework::Hist2<> h_decay_vz_vs_origin_vz{"/DecaysLambda/decay_vz_vs_origin_vz", "Decay vs origin vertex z;ov_{z} [cm];v_{z} [cm]", -30, 30, 100, -30, 30, 100};
    framework::Hist1<> h_vertex_detach{"/DecaysLambda/vertex_detach", "Vertex detachment (v_{z} - ov_{z});#Deltaz [cm];Events", -20, 40, 200};
    framework::Hist1<> h_vertex_detach_precut{"/DecaysLambda/vertex_detach_precut", "Vertex detachment (pre-cut);#Deltaz [cm];Events", -20, 40, 200};
    framework::Hist1<> h_vertex_detach_xy{"/DecaysLambda/vertex_detach_xy", "Transverse vertex detachment;#Deltar_{xy} [cm];Events", 0, 20, 100};
    struct DecaysLambdaNTupleData {
        std::vector<double> mass;
        std::vector<double> umass;
        std::vector<double> p;
        std::vector<double> theta;
        std::vector<double> phi;
        std::vector<double> decay_length;
        std::vector<double> vz;
        std::vector<double> ovz;
        std::vector<double> proton_p;
        std::vector<double> pion_p;
        std::vector<double> vertex_detach;

        static constexpr auto ntuple_fields() {
            using T = DecaysLambdaNTupleData;
            return std::make_tuple(framework::field("mass", &T::mass), framework::field("umass", &T::umass), framework::field("p", &T::p), framework::field("theta", &T::theta), framework::field("phi", &T::phi), framework::field("decay_length", &T::decay_length), framework::field("vz", &T::vz), framework::field("ovz", &T::ovz), framework::field("proton_p", &T::proton_p), framework::field("pion_p", &T::pion_p), framework::field("vertex_detach", &T::vertex_detach));
        }
    };

    framework::NTupleWriter<DecaysLambdaNTupleData> m_ntuple_writer;

    auto fill_lambda_histograms(const DecaysLambda& lambda, const std::vector<DecaysDaughter>& daughters) -> void {
        h_mass_selected.fill(lambda.mass);
        h_umass.fill(lambda.umass);
        h_momentum.fill(lambda.momentum());
        h_theta.fill(lambda.theta() * Constants::RAD_TO_DEG);
        h_phi.fill(lambda.phi() * Constants::RAD_TO_DEG);
        h_decay_length.fill(lambda.decay_length());
        h_decay_vz.fill(lambda.vz);
        h_origin_vz.fill(lambda.ovz);
        h_r.fill(lambda.r);
        h_ndau.fill(lambda.ndau);
        h_mass_vs_p.fill(lambda.momentum(), lambda.mass);
        h_decay_vz_vs_origin_vz.fill(lambda.ovz, lambda.vz);
        h_vertex_detach.fill(lambda.vertex_detach_z());
        h_vertex_detach_xy.fill(lambda.vertex_detach_xy());
        for (const auto& dau : daughters) {
            if (dau.idx == lambda.dau1idx || dau.idx == lambda.dau2idx) {
                if (dau.pid == static_cast<int>(PIDs::PROTON)) {
                    h_proton_p.fill(dau.momentum());
                } else if (dau.pid == static_cast<int>(PIDs::PIMINUS)) {
                    h_pion_p.fill(dau.momentum());
                }
            }
        }
    }

    auto fill_ntuple(const std::vector<DecaysLambda>& lambdas, const std::vector<DecaysDaughter>& daughters) -> void {
        auto& d = m_ntuple_writer.data();
        d.mass.clear();
        d.umass.clear();
        d.p.clear();
        d.theta.clear();
        d.phi.clear();
        d.decay_length.clear();
        d.vz.clear();
        d.ovz.clear();
        d.proton_p.clear();
        d.pion_p.clear();
        d.vertex_detach.clear();

        for (const auto& lambda : lambdas) {
            d.mass.push_back(lambda.mass);
            d.umass.push_back(lambda.umass);
            d.p.push_back(lambda.momentum());
            d.theta.push_back(lambda.theta());
            d.phi.push_back(lambda.phi());
            d.decay_length.push_back(lambda.decay_length());
            d.vz.push_back(lambda.vz);
            d.ovz.push_back(lambda.ovz);
            d.vertex_detach.push_back(lambda.vertex_detach_z());

            double proton_p = 0.0, pion_p = 0.0;
            for (const auto& dau : daughters) {
                if (dau.idx == lambda.dau1idx || dau.idx == lambda.dau2idx) {
                    if (dau.pid == static_cast<int>(PIDs::PROTON)) proton_p = dau.momentum();
                    else if (dau.pid == static_cast<int>(PIDs::PIMINUS)) pion_p = dau.momentum();
                }
            }
            d.proton_p.push_back(proton_p);
            d.pion_p.push_back(pion_p);
        }

        m_ntuple_writer.fill();
    }
};

}  // namespace algorithms
