#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include <clas12/Particle.hpp>
#include <clas12/Banks.hpp>
#include <clas12/ReadBank.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/services/LoggingService.hpp>

namespace clas12::framework {

/**
 * @brief Track quality level
 */
enum class TrackQuality {
    VeryLoose,  ///< Minimal requirements
    Loose,      ///< Standard loose selection
    Standard,   ///< Default quality
    Tight,      ///< High purity
    VeryTight   ///< Highest purity, lowest efficiency
};

/**
 * @brief Convert TrackQuality to string
 */
inline auto track_quality_to_string(TrackQuality q) -> std::string {
    switch (q) {
        case TrackQuality::VeryLoose: return "VeryLoose";
        case TrackQuality::Loose: return "Loose";
        case TrackQuality::Standard: return "Standard";
        case TrackQuality::Tight: return "Tight";
        case TrackQuality::VeryTight: return "VeryTight";
    }
    return "Unknown";
}

/**
 * @brief Detailed track quality information
 */
struct TrackQualityInfo {
    // DC quality
    int ndf = 0;                         ///< Number of degrees of freedom
    double chi2 = -1.0;                  ///< Chi-squared
    double chi2_ndf = -1.0;              ///< Chi2 per degree of freedom
    int n_superlayers = 0;               ///< Number of superlayers with hits
    std::array<double, 3> dc_edge{0, 0, 0};  ///< Edge distance for R1, R2, R3 [cm]

    // Track parameters
    int sector = 0;                      ///< Sector (1-6)
    double momentum = 0.0;               ///< Track momentum [GeV/c]
    double theta = 0.0;                  ///< Polar angle [degrees]
    double phi = 0.0;                    ///< Azimuthal angle [degrees]
    double vz = 0.0;                     ///< Vertex z [cm]

    // Status flags
    bool has_ftof = false;               ///< Has FTOF hit
    bool has_ctof = false;               ///< Has CTOF hit
    bool has_htcc = false;               ///< Has HTCC hit
    bool has_ecal = false;               ///< Has ECAL hit

    // Quality decisions
    bool passes_chi2 = false;            ///< Passes chi2/ndf cut
    bool passes_ndf = false;             ///< Passes NDF cut
    bool passes_fiducial = false;        ///< Passes fiducial cuts
    bool passes_vertex = false;          ///< Passes vertex cuts
    bool passes_all = false;             ///< Passes all cuts

    // ========================================================================
    // Quality Score
    // ========================================================================

    /**
     * @brief Compute overall quality score (0-1)
     *
     * Higher is better quality.
     */
    [[nodiscard]] auto quality_score() const -> double {
        double score = 0.0;
        int n_criteria = 0;

        // Chi2 contribution (lower is better)
        if (chi2_ndf > 0) {
            // Score decreases as chi2/ndf increases
            score += std::exp(-chi2_ndf / 10.0);
            ++n_criteria;
        }

        // NDF contribution (more is better)
        if (ndf > 0) {
            score += std::min(1.0, ndf / 30.0);  // Saturates at NDF=30
            ++n_criteria;
        }

        // DC edge contribution (larger distance is better)
        double min_edge = std::min({dc_edge[0], dc_edge[1], dc_edge[2]});
        if (min_edge > 0) {
            score += std::min(1.0, min_edge / 10.0);  // Saturates at 10 cm
            ++n_criteria;
        }

        // Detector hit contributions
        if (has_ftof) { score += 0.5; ++n_criteria; }
        if (has_ecal) { score += 0.5; ++n_criteria; }

        return (n_criteria > 0) ? score / n_criteria : 0.0;
    }

    /**
     * @brief Get minimum DC edge distance
     */
    [[nodiscard]] auto min_dc_edge() const -> double {
        return std::min({dc_edge[0], dc_edge[1], dc_edge[2]});
    }
};

/**
 * @brief Service for track quality assessment and selection
 *
 * TrackQualityService provides standardized track quality cuts
 * and assessment for CLAS12 analysis. It evaluates:
 * - Drift chamber fit quality (chi2, NDF)
 * - DC fiducial cuts (edge distances)
 * - Detector hit presence
 * - Vertex position
 *
 * Example usage:
 * @code
 * // In algorithm on_initialize():
 * m_track_svc = &require_service<TrackQualityService>("TrackQualityService");
 *
 * // In algorithm process():
 * auto electrons = event.particles(PIDs::ELECTRON);
 *
 * // Filter by quality
 * auto good_electrons = m_track_svc->filter_by_quality(
 *     electrons, event, TrackQuality::Standard);
 *
 * // Or assess individually
 * for (const auto& e : electrons) {
 *     auto quality = m_track_svc->assess_quality(e, event);
 *
 *     h_chi2_ndf->fill(quality.chi2_ndf);
 *     h_dc_edge->fill(quality.min_dc_edge());
 *
 *     if (quality.passes_all) {
 *         // Use track
 *     }
 * }
 * @endcode
 */
class TrackQualityService : public Service {
   public:
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "TrackQualityService"; }

    explicit TrackQualityService(std::string name = "TrackQualityService")
        : Service(std::move(name)) {}

    // ========================================================================
    // Service Lifecycle
    // ========================================================================

    auto on_initialize() -> StatusCode override {
        Log::info(this->name(), "TrackQualityService initialized");
        Log::info(this->name(), "  Chi2/ndf cuts: VeryLoose={}, Loose={}, Standard={}, Tight={}, VeryTight={}",
                  m_chi2_cut_veryloose.get(), m_chi2_cut_loose.get(),
                  m_chi2_cut_standard.get(), m_chi2_cut_tight.get(),
                  m_chi2_cut_verytight.get());
        Log::info(this->name(), "  Min NDF: {}", m_min_ndf.get());
        Log::info(this->name(), "  DC edge cut: {} cm", m_dc_edge_cut.get());
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        Log::info(this->name(), "TrackQualityService finalized");
        return StatusCode::SUCCESS();
    }

    // ========================================================================
    // Configuration Properties
    // ========================================================================

    /// Chi2/ndf cut for very loose quality
    Property<double> m_chi2_cut_veryloose{"chi2_cut_veryloose", 30.0,
                                          "Chi2/ndf cut for VeryLoose"};

    /// Chi2/ndf cut for loose quality
    Property<double> m_chi2_cut_loose{"chi2_cut_loose", 20.0,
                                      "Chi2/ndf cut for Loose"};

    /// Chi2/ndf cut for standard quality
    Property<double> m_chi2_cut_standard{"chi2_cut_standard", 10.0,
                                         "Chi2/ndf cut for Standard"};

    /// Chi2/ndf cut for tight quality
    Property<double> m_chi2_cut_tight{"chi2_cut_tight", 5.0,
                                      "Chi2/ndf cut for Tight"};

    /// Chi2/ndf cut for very tight quality
    Property<double> m_chi2_cut_verytight{"chi2_cut_verytight", 3.0,
                                          "Chi2/ndf cut for VeryTight"};

    /// Minimum NDF requirement
    Property<int> m_min_ndf{"min_ndf", 5, "Minimum number of degrees of freedom"};

    /// Minimum DC edge distance [cm]
    Property<double> m_dc_edge_cut{"dc_edge_cut", 5.0,
                                   "Minimum DC edge distance [cm]"};

    /// Require FTOF hit
    Property<bool> m_require_ftof{"require_ftof", false, "Require FTOF hit"};

    /// Vertex z range
    Property<double> m_vz_min{"vz_min", -15.0, "Minimum vertex z [cm]"};
    Property<double> m_vz_max{"vz_max", 5.0, "Maximum vertex z [cm]"};

    // ========================================================================
    // Quality Assessment
    // ========================================================================

    /**
     * @brief Get full track quality information
     *
     * @param track Track particle
     * @param event Event for bank access
     * @return TrackQualityInfo with all quality metrics
     */
    auto assess_quality(const clas12::Particle& track, Event& event)
        -> TrackQualityInfo {
        TrackQualityInfo info;

        // Basic track parameters
        info.momentum = track.p();
        info.theta = track.theta();
        info.phi = track.phi();
        info.vz = track.vz();
        info.sector = determine_sector(track);

        // Read DC track information
        read_dc_info(track, event, info);

        // Check detector hits
        check_detector_hits(track, event, info);

        // Apply quality cuts
        info.passes_chi2 = (info.chi2_ndf > 0 && info.chi2_ndf < m_chi2_cut_standard.get());
        info.passes_ndf = (info.ndf >= m_min_ndf.get());
        info.passes_fiducial = (info.min_dc_edge() > m_dc_edge_cut.get());
        info.passes_vertex = (info.vz >= m_vz_min.get() && info.vz <= m_vz_max.get());

        // Overall decision
        info.passes_all = info.passes_chi2 && info.passes_ndf &&
                          info.passes_fiducial && info.passes_vertex;

        if (m_require_ftof.get()) {
            info.passes_all = info.passes_all && info.has_ftof;
        }

        return info;
    }

    /**
     * @brief Check if track passes quality level
     *
     * @param track Track particle
     * @param event Event
     * @param level Quality level to check
     * @return True if track passes all cuts for this level
     */
    [[nodiscard]] auto passes_quality(const clas12::Particle& track,
                                      Event& event,
                                      TrackQuality level = TrackQuality::Standard) -> bool {
        auto info = assess_quality(track, event);

        // Get chi2 cut for this level
        double chi2_cut = get_chi2_cut(level);

        // Check all criteria
        bool passes_chi2 = (info.chi2_ndf > 0 && info.chi2_ndf < chi2_cut);
        bool passes_ndf = (info.ndf >= m_min_ndf.get());
        bool passes_fid = (info.min_dc_edge() > m_dc_edge_cut.get());
        bool passes_vz = (info.vz >= m_vz_min.get() && info.vz <= m_vz_max.get());

        bool passes = passes_chi2 && passes_ndf && passes_fid && passes_vz;

        // Tighter levels may have additional requirements
        if (level >= TrackQuality::Tight) {
            passes = passes && info.has_ftof;
        }

        return passes;
    }

    /**
     * @brief Get quality score (0-1)
     *
     * @param track Track particle
     * @param event Event
     * @return Quality score (higher is better)
     */
    [[nodiscard]] auto quality_score(const clas12::Particle& track, Event& event)
        -> double {
        auto info = assess_quality(track, event);
        return info.quality_score();
    }

    // ========================================================================
    // Individual Cuts
    // ========================================================================

    /**
     * @brief Check chi2/ndf cut
     */
    [[nodiscard]] auto passes_chi2_cut(const clas12::Particle& track,
                                       Event& event,
                                       double max_chi2_ndf) -> bool {
        auto info = assess_quality(track, event);
        return (info.chi2_ndf > 0 && info.chi2_ndf < max_chi2_ndf);
    }

    /**
     * @brief Check NDF cut
     */
    [[nodiscard]] auto passes_ndf_cut(const clas12::Particle& track,
                                      Event& event,
                                      int min_ndf) -> bool {
        auto info = assess_quality(track, event);
        return (info.ndf >= min_ndf);
    }

    /**
     * @brief Check DC fiducial cut
     */
    [[nodiscard]] auto passes_dc_fiducial(const clas12::Particle& track,
                                          Event& event,
                                          double min_edge) -> bool {
        auto info = assess_quality(track, event);
        return (info.min_dc_edge() > min_edge);
    }

    /**
     * @brief Check vertex z cut
     */
    [[nodiscard]] auto passes_vertex_cut(const clas12::Particle& track,
                                         double vz_min, double vz_max) -> bool {
        return (track.vz() >= vz_min && track.vz() <= vz_max);
    }

    // ========================================================================
    // Filtering
    // ========================================================================

    /**
     * @brief Filter particles by quality
     *
     * @param tracks Vector of particles
     * @param event Event
     * @param level Quality level requirement
     * @return Vector of particles passing quality cuts
     */
    auto filter_by_quality(const std::vector<clas12::Particle>& tracks,
                          Event& event,
                          TrackQuality level = TrackQuality::Standard)
        -> std::vector<clas12::Particle> {
        std::vector<clas12::Particle> result;
        result.reserve(tracks.size());

        for (const auto& track : tracks) {
            if (passes_quality(track, event, level)) {
                result.push_back(track);
            }
        }

        return result;
    }

    /**
     * @brief Get best track (highest quality score)
     *
     * @param tracks Vector of particles
     * @param event Event
     * @return Optional containing best track, or empty if none
     */
    auto best_track(const std::vector<clas12::Particle>& tracks,
                   Event& event) -> std::optional<clas12::Particle> {
        if (tracks.empty()) {
            return std::nullopt;
        }

        double best_score = -1.0;
        const clas12::Particle* best = nullptr;

        for (const auto& track : tracks) {
            double score = quality_score(track, event);
            if (score > best_score) {
                best_score = score;
                best = &track;
            }
        }

        if (best) {
            return *best;
        }
        return std::nullopt;
    }

    /**
     * @brief Sort tracks by quality (best first)
     */
    auto sort_by_quality(std::vector<clas12::Particle>& tracks, Event& event) -> void {
        // Cache quality scores
        std::vector<std::pair<double, size_t>> scores;
        scores.reserve(tracks.size());

        for (size_t i = 0; i < tracks.size(); ++i) {
            scores.emplace_back(quality_score(tracks[i], event), i);
        }

        // Sort by score (descending)
        std::sort(scores.begin(), scores.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });

        // Reorder tracks
        std::vector<clas12::Particle> sorted;
        sorted.reserve(tracks.size());
        for (const auto& [score, idx] : scores) {
            sorted.push_back(tracks[idx]);
        }

        tracks = std::move(sorted);
    }

   private:
    /**
     * @brief Determine sector from phi angle
     */
    auto determine_sector(const clas12::Particle& track) const -> int {
        double phi = track.phi();  // degrees

        // Normalize to [0, 360)
        while (phi < 0) phi += 360.0;
        while (phi >= 360.0) phi -= 360.0;

        // Sectors are 60 degrees wide, centered at 0, 60, 120, 180, 240, 300
        int sector = static_cast<int>((phi + 30.0) / 60.0) % 6 + 1;
        return sector;
    }

    /**
     * @brief Read DC track information from banks
     */
    auto read_dc_info(const clas12::Particle& track, Event& event,
                      TrackQualityInfo& info) -> void {
        // Try to read REC::Track bank
        if (event.has_bank("REC::Track")) {
            const auto& track_bank = event.get_bank("REC::Track");

            for (int i = 0; i < track_bank.rows(); ++i) {
                int pindex = static_cast<int>(track_bank.get<int16_t>("pindex", i));
                if (pindex != track.index()) continue;

                info.chi2 = track_bank.get<float>("chi2", i);
                info.ndf = static_cast<int>(track_bank.get<int16_t>("NDF", i));
                info.sector = static_cast<int>(track_bank.get<int8_t>("sector", i));

                if (info.ndf > 0) {
                    info.chi2_ndf = info.chi2 / info.ndf;
                }
                break;
            }
        }

        // Try to read REC::Traj for DC edge distances
        if (event.has_bank("REC::Traj")) {
            const auto& traj_bank = event.get_bank("REC::Traj");

            for (int i = 0; i < traj_bank.rows(); ++i) {
                int pindex = static_cast<int>(traj_bank.get<int16_t>("pindex", i));
                if (pindex != track.index()) continue;

                int detector = static_cast<int>(traj_bank.get<int8_t>("detector", i));
                int layer = static_cast<int>(traj_bank.get<int8_t>("layer", i));

                // DC detector ID is 6
                if (detector == 6) {
                    double edge = static_cast<double>(traj_bank.get<float>("edge", i));

                    // Map layer to region
                    if (layer == 6) info.dc_edge[0] = edge;       // R1
                    else if (layer == 18) info.dc_edge[1] = edge; // R2
                    else if (layer == 36) info.dc_edge[2] = edge; // R3
                }
            }
        }
    }

    /**
     * @brief Check for detector hits
     */
    auto check_detector_hits(const clas12::Particle& track, Event& event,
                             TrackQualityInfo& info) -> void {
        // Check FTOF
        if (event.has_bank("REC::Scintillator")) {
            const auto& scint_bank = event.get_bank("REC::Scintillator");
            for (int i = 0; i < scint_bank.rows(); ++i) {
                int pindex = static_cast<int>(scint_bank.get<int16_t>("pindex", i));
                if (pindex != track.index()) continue;

                int detector = static_cast<int>(scint_bank.get<int8_t>("detector", i));
                if (detector == 12) {  // FTOF
                    info.has_ftof = true;
                } else if (detector == 4) {  // CTOF
                    info.has_ctof = true;
                }
            }
        }

        // Check HTCC
        if (event.has_bank("REC::Cherenkov")) {
            const auto& cher_bank = event.get_bank("REC::Cherenkov");
            for (int i = 0; i < cher_bank.rows(); ++i) {
                int pindex = static_cast<int>(cher_bank.get<int16_t>("pindex", i));
                if (pindex != track.index()) continue;

                int detector = static_cast<int>(cher_bank.get<int8_t>("detector", i));
                if (detector == 15) {  // HTCC
                    info.has_htcc = true;
                    break;
                }
            }
        }

        // Check ECAL
        if (event.has_bank("REC::Calorimeter")) {
            const auto& calo_bank = event.get_bank("REC::Calorimeter");
            for (int i = 0; i < calo_bank.rows(); ++i) {
                int pindex = static_cast<int>(calo_bank.get<int16_t>("pindex", i));
                if (pindex != track.index()) continue;

                int detector = static_cast<int>(calo_bank.get<int8_t>("detector", i));
                if (detector == 7) {  // ECAL
                    info.has_ecal = true;
                    break;
                }
            }
        }
    }

    /**
     * @brief Get chi2/ndf cut for quality level
     */
    [[nodiscard]] auto get_chi2_cut(TrackQuality level) const -> double {
        switch (level) {
            case TrackQuality::VeryLoose: return m_chi2_cut_veryloose.get();
            case TrackQuality::Loose: return m_chi2_cut_loose.get();
            case TrackQuality::Standard: return m_chi2_cut_standard.get();
            case TrackQuality::Tight: return m_chi2_cut_tight.get();
            case TrackQuality::VeryTight: return m_chi2_cut_verytight.get();
        }
        return m_chi2_cut_standard.get();
    }
};

}  // namespace clas12::framework
