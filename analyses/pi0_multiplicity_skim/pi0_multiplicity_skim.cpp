/**
 * @file pi0_multiplicity_skim.cpp
 * @brief Slim skim for pi0 multiplicity analysis: electron + DIS + good photon(s)
 *
 * Pre-filters events requiring:
 * 1. Good electron (chi2pid, momentum, vz, sampling fraction, fiducial)
 * 2. DIS kinematics (Q2 >= 1, W >= 2, y <= 0.85)
 * 3. At least one good photon (AI cuts + beta + PCAL fiducial)
 *
 * Instead of a raw passthrough of every input bank (which produced 200-300 GB
 * slices), passing events are reduced to two tiny custom banks holding only
 * what Pi0AnalysisAlg consumes:
 *   - PI0::event   (1 row):  run, event, helicity, DIS Q2/xB/nu/W/y, electron 4-vec
 *   - PI0::photons (N rows): selected photon px/py/pz/E (Catboost AI already applied)
 *
 * pi0 candidates are re-derived downstream from the photon 4-vectors, so the
 * skim runs the expensive selection chain (esp. the photon AI) ONCE and the
 * analysis reads the slim banks directly — ~20-50x smaller and much faster.
 */

#include <string>

#include <clas12/framework/core/AnalysisRunner.hpp>
#include <clas12/framework/core/EventHelpers.hpp>
#include <clas12/framework/core/TypedEventStore.hpp>
#include <clas12/algorithms/selection/DISSelectionAlg.hpp>
#include <clas12/algorithms/selection/ElectronSelectionAlg.hpp>
#include <clas12/algorithms/selection/PhotonSelectionAlg.hpp>
#include <clas12/algorithms/monitoring/EventMonitor.hpp>
#include <clas12/services/ElectronCutsService.hpp>
#include <clas12/services/PhotonCutsService.hpp>
#include <clas12/services/VzCorrectorService.hpp>
#include <clas12/framework/services/HipoFileService.hpp>
#include <clas12/physics/DISAnalysis.hpp>
#include <clas12/Particle.hpp>
#include <clas12/bankdefs/PI0_event.hpp>
#include <clas12/bankdefs/PI0_photons.hpp>
#include <clas12/bankdefs/REC_Event.hpp>
#include <clas12/bankdefs/RUN_config.hpp>

namespace {

/**
 * @brief Build a runtime hipo::schema from a generated bank wrapper.
 *
 * The wrapper carries `bank_name`, `bank_group`, `bank_item` and the
 * `bank_schema` format string ("name/Type,...") emitted by gen_bankdefs.py —
 * so the writer and the reader share one source of truth (the JSON bankdef).
 */
template <typename BankDef>
auto make_schema() -> hipo::schema {
    hipo::schema s(std::string(BankDef::bank_name).c_str(),
                   BankDef::bank_group, BankDef::bank_item);
    s.parse(std::string(BankDef::bank_schema));
    return s;
}

/**
 * @brief Algorithm that writes each passing event as slim PI0 banks.
 *
 * Placed last in the algorithm chain. Events that reach this algorithm have
 * passed all upstream cuts (electron + DIS + photon selection). It reads the
 * typed event store (DIS kinematics, trigger electron, selected photons) plus
 * REC::Event helicity / RUN::config run number, fills PI0::event (1 row) and
 * PI0::photons (N rows), and hands the assembled hipo::event to the
 * HipoFileService writer.
 */
class SlimWriterAlg : public framework::Algorithm {
public:
    explicit SlimWriterAlg(std::string name = "SlimWriterAlg")
        : Algorithm(std::move(name)) {}

    auto on_initialize() -> framework::StatusCode override {
        m_hipo_svc = svc<framework::HipoFileService>();
        if (!m_hipo_svc || !m_hipo_svc->is_output_enabled()) {
            framework::Log::warning(name(), "HIPO output not configured — slim writer disabled");
            m_enabled = false;
            return framework::StatusCode::SUCCESS();
        }
        m_enabled = true;

        // REC::Event (helicity) and RUN::config (run number) are the only raw
        // banks this writer reads directly; everything else comes from the
        // typed store populated upstream.
        declare_banks_typed<bankdefs::REC_Event, bankdefs::RUN_config>();

        // Schemas used to construct the output banks per event. The matching
        // schemas are registered with the HipoFileService in main() (before
        // the output file is opened) so the file's dictionary carries them.
        m_event_schema  = make_schema<bankdefs::PI0_event>();
        m_photon_schema = make_schema<bankdefs::PI0_photons>();

        framework::Log::info(name(), "Will write slim PI0::event + PI0::photons banks");
        return framework::StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> framework::StatusCode override {
        if (!m_enabled) return framework::StatusCode::SUCCESS();

        // Read everything the analysis needs from the typed store. The chain
        // upstream only lets electron+DIS+photon events through, but guard
        // defensively so a missing key skips the event rather than writing junk.
        auto dis = framework::EventHelpers::get_dis_kinematics(event);
        if (!dis) return framework::StatusCode::SUCCESS();

        auto electron_opt = event.retrieve_typed(framework::Keys::TriggerElectron);
        if (!electron_opt) return framework::StatusCode::SUCCESS();

        auto photons_opt = event.retrieve_typed(framework::Keys::SelectedPhotons_Pi0);
        if (!photons_opt || photons_opt->empty()) return framework::StatusCode::SUCCESS();
        const auto& photons = *photons_opt;

        auto& rec_event = event.bank<bankdefs::REC_Event>();
        const int helicity = rec_event.rows() > 0 ? rec_event.helicity(0) : 0;

        auto& run_config = event.bank<bankdefs::RUN_config>();
        const int run = run_config.rows() > 0 ? run_config.run(0) : 0;

        const long event_id = event.event_number();
        const auto e4 = electron_opt->PxPyPzEVector();

        hipo::event ev;

        hipo::bank ev_bank(m_event_schema, 1);
        ev_bank.putInt("run", 0, run);
        ev_bank.putLong("event", 0, static_cast<int64_t>(event_id));
        ev_bank.putInt("helicity", 0, helicity);
        ev_bank.putFloat("Q2", 0, static_cast<float>(dis->Q2));
        ev_bank.putFloat("xB", 0, static_cast<float>(dis->xB));
        ev_bank.putFloat("nu", 0, static_cast<float>(dis->nu));
        ev_bank.putFloat("W",  0, static_cast<float>(dis->W));
        ev_bank.putFloat("y",  0, static_cast<float>(dis->y));
        ev_bank.putFloat("ex", 0, static_cast<float>(e4.Px()));
        ev_bank.putFloat("ey", 0, static_cast<float>(e4.Py()));
        ev_bank.putFloat("ez", 0, static_cast<float>(e4.Pz()));
        ev_bank.putFloat("eE", 0, static_cast<float>(e4.E()));
        ev.addStructure(ev_bank);

        const int n_photons = static_cast<int>(photons.size());
        hipo::bank ph_bank(m_photon_schema, n_photons);
        for (int i = 0; i < n_photons; ++i) {
            ph_bank.putFloat("px", i, static_cast<float>(photons[i].px()));
            ph_bank.putFloat("py", i, static_cast<float>(photons[i].py()));
            ph_bank.putFloat("pz", i, static_cast<float>(photons[i].pz()));
            ph_bank.putFloat("E",  i, static_cast<float>(photons[i].E()));
        }
        ev.addStructure(ph_bank);

        m_hipo_svc->write_event(ev);
        return framework::StatusCode::SUCCESS();
    }

    auto on_finalize() -> framework::StatusCode override {
        if (m_enabled && m_hipo_svc) {
            framework::Log::info(name(), "Slim events written to HIPO: {}",
                                          m_hipo_svc->events_written());
        }
        return framework::StatusCode::SUCCESS();
    }

private:
    framework::HipoFileService* m_hipo_svc = nullptr;
    bool m_enabled = false;
    hipo::schema m_event_schema;
    hipo::schema m_photon_schema;
};

}  // namespace

auto main(int argc, char* argv[]) -> int {
    return framework::AnalysisRunner::run(
        argc, argv, "Pi0 Multiplicity Skim", "pi0_multiplicity_skim",
        [](auto& builder, const auto& config) {
            // Optional HIPO output file path. The CLI override
            // (`--hipo-output FILE`) wins over `[skim].hipo_output` in the
            // TOML so local_batch can hand each slice a unique skim file
            // without rewriting the TOML.
            std::string hipo_output = config.hipo_output_override;
            if (hipo_output.empty() && config.config_table) {
                if (auto v = (*config.config_table)["skim"]["hipo_output"].template value<std::string>())
                    hipo_output = *v;
            }

            builder
                .template add_service<framework::VzCorrectorService>(framework::VzCorrectorService::Config{.target = config.target, .polarity = config.polarity})
                .template add_service<framework::ElectronCutsService>()
                .template add_service<framework::PhotonCutsService>()
                .template add_algorithm<algorithms::EventMonitor>("EventMonitor")
                .template add_algorithm<algorithms::ElectronSelectionAlg>("ElectronSelectionAlg", algorithms::ElectronSelectionConfig{.target = config.target, .polarity = config.polarity})
                .template add_algorithm<algorithms::DISSelectionAlg>("DISSelection", algorithms::DISSelectionConfig{.dis_region = physics::DISAnalysis::standard_dis_region()})
                .template add_algorithm<algorithms::PhotonSelectionAlg>("PhotonSelection");

            if (!hipo_output.empty()) {
                builder.template configure_service<framework::HipoFileService>(
                    [hipo_output](framework::HipoFileService& svc) {
                        svc.set_output_file(hipo_output);
                        // Register the slim bank schemas BEFORE the output
                        // file is opened (services initialize before algorithm
                        // on_initialize), so the file's dictionary carries
                        // PI0::event / PI0::photons and the writer can
                        // serialize them.
                        svc.register_output_schema(make_schema<bankdefs::PI0_event>());
                        svc.register_output_schema(make_schema<bankdefs::PI0_photons>());
                    });
                builder.template add_algorithm<SlimWriterAlg>("SlimWriter");
            }
        });
}
