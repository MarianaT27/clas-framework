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
#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>
#include <clas12/framework/services/NTupleService.hpp>

namespace clas12::algorithms {

using clas12::framework::Log;

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
};

/**
 * @brief Algorithm for Lambda analysis using DECAYS::Particle bank
 */
class DecaysLambdaAlg : public clas12::framework::Algorithm {
   public:
    using Config = DecaysLambdaConfig;
    using StatusCode = clas12::framework::StatusCode;

    explicit DecaysLambdaAlg(std::string name = "DecaysLambdaAlg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config), m_cutflow("DECAYS Lambda Analysis") {
        required_services(m_hist, m_ntuple);
        declare_banks("DECAYS::Particle");
        declare_properties(m_mass_window, m_min_momentum, m_min_decay_length, m_min_vertex_detach, m_max_vertex_detach, m_enable_mass_window_cut, m_enable_momentum_cut, m_enable_decay_length_cut, m_enable_vertex_detach_cut);
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        Log::subsection(fmt::format("{} Configuration", name()));
        Log::info(name(), "Initializing DECAYS Lambda analysis algorithm");
        Log::info(name(), "  Mass window: {} GeV", m_mass_window.get());
        Log::info(name(), "  Min momentum: {} GeV", m_min_momentum.get());
        Log::info(name(), "  Min decay length: {} cm", m_min_decay_length.get());
        Log::info(name(), "  Min vertex detach: {} cm", m_min_vertex_detach.get());
        Log::info(name(), "  Max vertex detach: {} cm", m_max_vertex_detach.get());

        book_histograms();
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

    auto process(clas12::framework::Event& event) -> StatusCode override {
        m_cutflow.pass("All Events");

        clas12::bankdefs::DECAYS_Particle decays;
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

        if (h_n_lambdas) h_n_lambdas->fill(lambdas.size());
        for (const auto& lambda : lambdas) {
            if (h_mass_nocuts) h_mass_nocuts->fill(lambda.mass);
        }

        std::vector<DecaysLambda> selected_lambdas;
        bool passed_mass = false;
        bool passed_momentum = false;
        bool passed_decay_length = false;
        bool passed_vertex_detach = false;

        for (const auto& lambda : lambdas) {
            if (h_vertex_detach_precut) h_vertex_detach_precut->fill(lambda.vertex_detach_z());

            if (m_enable_mass_window_cut.get() && std::abs(lambda.mass - LAMBDA_MASS_PDG) > m_mass_window.get()) continue;
            passed_mass = true;

            if (m_enable_momentum_cut.get() && lambda.momentum() < m_min_momentum.get()) continue;
            passed_momentum = true;

            if (m_enable_decay_length_cut.get() && m_min_decay_length.get() > 0 && lambda.decay_length() < m_min_decay_length.get()) continue;
            passed_decay_length = true;

            if (m_enable_vertex_detach_cut.get() && m_min_vertex_detach.get() > -999) {
                double detach = lambda.vertex_detach_z();
                if (detach < m_min_vertex_detach.get()) continue;
                if (m_max_vertex_detach.get() > 0 && detach > m_max_vertex_detach.get()) continue;
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

        event.set_decision_typed(clas12::framework::Decisions::HasDecaysLambda, true);

        auto best_it = std::min_element(selected_lambdas.begin(), selected_lambdas.end(), [](const DecaysLambda& a, const DecaysLambda& b) { return std::abs(a.mass - LAMBDA_MASS_PDG) < std::abs(b.mass - LAMBDA_MASS_PDG); });
        event.store_typed(clas12::framework::Keys::DecaysLambdaMass, best_it->mass);
        event.store_typed(clas12::framework::Keys::DecaysLambdaMomentum, best_it->momentum());

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

    clas12::framework::Property<double> m_mass_window{"mass_window", 0.1, "Mass window around PDG mass [GeV]"};
    clas12::framework::Property<double> m_min_momentum{"min_momentum", 0.0, "Minimum Lambda momentum [GeV]"};
    clas12::framework::Property<double> m_min_decay_length{"min_decay_length", 0.0, "Minimum decay length [cm], 0=disabled"};
    clas12::framework::Property<double> m_min_vertex_detach{"min_vertex_detach", -1000.0, "Minimum z displacement [cm], -1000=disabled"};
    clas12::framework::Property<double> m_max_vertex_detach{"max_vertex_detach", 0.0, "Maximum z displacement [cm], 0=no limit"};
    clas12::framework::Property<bool> m_enable_mass_window_cut{"enable_mass_window_cut", true, "Enable mass window cut"};
    clas12::framework::Property<bool> m_enable_momentum_cut{"enable_momentum_cut", true, "Enable momentum cut"};
    clas12::framework::Property<bool> m_enable_decay_length_cut{"enable_decay_length_cut", true, "Enable decay length cut"};
    clas12::framework::Property<bool> m_enable_vertex_detach_cut{"enable_vertex_detach_cut", true, "Enable vertex detachment cut"};
    clas12::framework::Cutflow m_cutflow;

    clas12::framework::ServiceRef<clas12::framework::HistogramService> m_hist;
    clas12::framework::ServiceRef<clas12::framework::NTupleService> m_ntuple;

    clas12::framework::ThreadLocalHistogram1D* h_n_lambdas = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_mass_nocuts = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_mass_selected = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_umass = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_momentum = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_theta = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_phi = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_decay_length = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_decay_vz = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_origin_vz = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_r = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_ndau = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_proton_p = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_pion_p = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_mass_vs_p = nullptr;
    clas12::framework::ThreadLocalHistogram2D* h_decay_vz_vs_origin_vz = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vertex_detach = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vertex_detach_precut = nullptr;
    clas12::framework::ThreadLocalHistogram1D* h_vertex_detach_xy = nullptr;

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
            return std::make_tuple(clas12::framework::field("mass", &T::mass), clas12::framework::field("umass", &T::umass), clas12::framework::field("p", &T::p), clas12::framework::field("theta", &T::theta), clas12::framework::field("phi", &T::phi), clas12::framework::field("decay_length", &T::decay_length), clas12::framework::field("vz", &T::vz), clas12::framework::field("ovz", &T::ovz), clas12::framework::field("proton_p", &T::proton_p), clas12::framework::field("pion_p", &T::pion_p), clas12::framework::field("vertex_detach", &T::vertex_detach));
        }
    };

    clas12::framework::NTupleWriter<DecaysLambdaNTupleData> m_ntuple_writer;

    auto book_histograms() -> void {
        h_n_lambdas = m_hist->book_hist1D("/DecaysLambda/n_lambdas", "Number of #Lambda per event;N_{#Lambda};Events", 10, 0, 10);
        h_mass_nocuts = m_hist->book_hist1D("/DecaysLambda/mass_nocuts", "#Lambda mass (no cuts);M [GeV];Events", 200, 1.0, 1.3);
        h_mass_selected = m_hist->book_hist1D("/DecaysLambda/mass_selected", "#Lambda mass (selected);M [GeV];Events", 200, 1.0, 1.3);
        h_umass = m_hist->book_hist1D("/DecaysLambda/umass", "#Lambda unmeasured mass;M [GeV];Events", 200, 1.0, 1.3);
        h_momentum = m_hist->book_hist1D("/DecaysLambda/momentum", "#Lambda momentum;p [GeV];Events", 100, 0, 10);
        h_theta = m_hist->book_hist1D("/DecaysLambda/theta", "#Lambda #theta;#theta [deg];Events", 90, 0, 90);
        h_phi = m_hist->book_hist1D("/DecaysLambda/phi", "#Lambda #phi;#phi [deg];Events", 180, -180, 180);
        h_decay_length = m_hist->book_hist1D("/DecaysLambda/decay_length", "#Lambda decay length;L [cm];Events", 100, 0, 50);
        h_decay_vz = m_hist->book_hist1D("/DecaysLambda/decay_vz", "#Lambda decay vertex z;v_{z} [cm];Events", 200, -30, 30);
        h_origin_vz = m_hist->book_hist1D("/DecaysLambda/origin_vz", "#Lambda origin vertex z;ov_{z} [cm];Events", 200, -30, 30);
        h_r = m_hist->book_hist1D("/DecaysLambda/r", "#Lambda decay radius;r [cm];Events", 100, 0, 20);
        h_ndau = m_hist->book_hist1D("/DecaysLambda/ndau", "Number of daughters;N_{dau};Events", 5, 0, 5);
        h_proton_p = m_hist->book_hist1D("/DecaysLambda/proton_p", "Proton momentum;p [GeV];Events", 100, 0, 5);
        h_pion_p = m_hist->book_hist1D("/DecaysLambda/pion_p", "#pi^{-} momentum;p [GeV];Events", 100, 0, 3);
        h_mass_vs_p = m_hist->book_hist2D("/DecaysLambda/mass_vs_p", "#Lambda mass vs momentum;p [GeV];M [GeV]", 100, 0, 10, 100, 1.0, 1.3);
        h_decay_vz_vs_origin_vz = m_hist->book_hist2D("/DecaysLambda/decay_vz_vs_origin_vz", "Decay vs origin vertex z;ov_{z} [cm];v_{z} [cm]", 100, -30, 30, 100, -30, 30);
        h_vertex_detach = m_hist->book_hist1D("/DecaysLambda/vertex_detach", "Vertex detachment (v_{z} - ov_{z});#Deltaz [cm];Events", 200, -20, 40);
        h_vertex_detach_precut = m_hist->book_hist1D("/DecaysLambda/vertex_detach_precut", "Vertex detachment (pre-cut);#Deltaz [cm];Events", 200, -20, 40);
        h_vertex_detach_xy = m_hist->book_hist1D("/DecaysLambda/vertex_detach_xy", "Transverse vertex detachment;#Deltar_{xy} [cm];Events", 100, 0, 20);
    }

    auto fill_lambda_histograms(const DecaysLambda& lambda, const std::vector<DecaysDaughter>& daughters) -> void {
        if (h_mass_selected) h_mass_selected->fill(lambda.mass);
        if (h_umass) h_umass->fill(lambda.umass);
        if (h_momentum) h_momentum->fill(lambda.momentum());
        if (h_theta) h_theta->fill(lambda.theta() * clas12::Constants::RAD_TO_DEG);
        if (h_phi) h_phi->fill(lambda.phi() * clas12::Constants::RAD_TO_DEG);
        if (h_decay_length) h_decay_length->fill(lambda.decay_length());
        if (h_decay_vz) h_decay_vz->fill(lambda.vz);
        if (h_origin_vz) h_origin_vz->fill(lambda.ovz);
        if (h_r) h_r->fill(lambda.r);
        if (h_ndau) h_ndau->fill(lambda.ndau);
        if (h_mass_vs_p) h_mass_vs_p->fill(lambda.momentum(), lambda.mass);
        if (h_decay_vz_vs_origin_vz) h_decay_vz_vs_origin_vz->fill(lambda.ovz, lambda.vz);
        if (h_vertex_detach) h_vertex_detach->fill(lambda.vertex_detach_z());
        if (h_vertex_detach_xy) h_vertex_detach_xy->fill(lambda.vertex_detach_xy());

        for (const auto& dau : daughters) {
            if (dau.idx == lambda.dau1idx || dau.idx == lambda.dau2idx) {
                if (dau.pid == static_cast<int>(clas12::PIDs::PROTON)) {
                    if (h_proton_p) h_proton_p->fill(dau.momentum());
                } else if (dau.pid == static_cast<int>(clas12::PIDs::PIMINUS)) {
                    if (h_pion_p) h_pion_p->fill(dau.momentum());
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
                    if (dau.pid == static_cast<int>(clas12::PIDs::PROTON)) proton_p = dau.momentum();
                    else if (dau.pid == static_cast<int>(clas12::PIDs::PIMINUS)) pion_p = dau.momentum();
                }
            }
            d.proton_p.push_back(proton_p);
            d.pion_p.push_back(pion_p);
        }

        m_ntuple_writer.fill();
    }
};

}  // namespace clas12::algorithms
