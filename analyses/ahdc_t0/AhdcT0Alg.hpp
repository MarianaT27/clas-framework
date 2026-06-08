#pragma once

/**
 * @file AhdcT0Alg.hpp
 * @brief Per-wire t0 (timing pedestal) extraction for the AHDC from the
 *        uncalibrated `AHDC::adc` bank.
 *
 * The AHDC (ALERT Hyperbolic Drift Chamber) raw bank `AHDC::adc` carries three
 * per-hit timing pickoffs — `time` (time at the ADC maximum), `leadingEdgeTime`
 * (time at half amplitude) and `constantFractionTime` (CFD) — all uncalibrated.
 * The t0 is the leading edge / start of each wire's timing spectrum; it is
 * extracted offline by fitting the per-wire histograms this algorithm produces.
 *
 * Two samples are booked per wire:
 *   - `all`      : every good AHDC::adc hit on that wire.
 *   - `on_track` : only hits that belong to a reconstructed track.
 *
 * `AHDC::adc` has no `trackid` column, so "on track" is decided by a per-event
 * geometry join against the calibrated `AHDC::hits` bank (which does carry
 * `trackid`, -1 when the hit is not on a track). The join key is the channel
 * identity (superlayer, layer, wire); `AHDC::hits` has no sector column, so
 * sector is intentionally not part of the key.
 *
 * Track-quality cuts gate the `on_track` sample: the per-track quality variables
 * (n_hits, chi2/ndf, ...) come from the reconstructed `AHDC::kftrack` bank and
 * are histogrammed (under `/AHDC/quality/`) for every track BEFORE the cuts, so
 * the thresholds can be read off the plots. A hit stays on-track only if its KF
 * track passes every enabled cut (a cut threshold of 0 disables that cut).
 */

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <fmt/format.h>

#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/Event.hpp>
#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/core/Property.hpp>
#include <clas12/framework/core/StatusCode.hpp>
#include <clas12/framework/services/HistogramService.hpp>
#include <clas12/framework/services/LoggingService.hpp>

#include <clas12/bankdefs/AHDC_adc.hpp>
#include <clas12/bankdefs/AHDC_hits.hpp>
#include <clas12/bankdefs/AHDC_kftrack.hpp>

#include <mh/mh.hpp>

namespace ahdc_t0 {

using framework::Log;

/// Canonical AHDC geometry: one entry per layer. The packed `AHDC::adc.layer`
/// value is `superlayer*10 + layer`; wires are numbered 1..nwires. Total 576.
struct AhdcLayer {
    int superlayer;
    int layer;
    int nwires;
};
static constexpr std::array<AhdcLayer, 8> kAhdcGeom = {{
    {1, 1, 47},
    {2, 1, 56},
    {2, 2, 56},
    {3, 1, 72},
    {3, 2, 72},
    {4, 1, 87},
    {4, 2, 87},
    {5, 1, 99},
}};

/// Channel key shared by AHDC::adc and AHDC::hits: packed_layer*1000 + wire,
/// where packed_layer = superlayer*10 + layer. Ranges are tiny (packed <= 51,
/// wire <= 99), so this never collides and fits an int.
[[nodiscard]] inline int channel_key(int packed_layer, int wire) noexcept {
    return packed_layer * 1000 + wire;
}

/// One {time, leadingEdgeTime, constantFractionTime} histogram triad.
/// Heap-allocated so each histogram has a stable address for register_hist().
struct ColumnSet {
    std::unique_ptr<framework::H1<>> time;
    std::unique_ptr<framework::H1<>> let;
    std::unique_ptr<framework::H1<>> cft;

    void create(double lo, double hi, std::size_t n) {
        time = std::make_unique<framework::H1<>>(mh::regular(lo, hi, n));
        let  = std::make_unique<framework::H1<>>(mh::regular(lo, hi, n));
        cft  = std::make_unique<framework::H1<>>(mh::regular(lo, hi, n));
    }
    void fill(float t, float l, float c) const {
        time->fill(t);
        let->fill(l);
        cft->fill(c);
    }
};

/// The two samples (all hits / on-track hits) for one channel (or the global
/// summed spectra).
struct SampleHists {
    ColumnSet all;
    ColumnSet on_track;
};

struct AhdcT0Config;  // forward declaration for TrackQualityHists::create.

/// Per-track quality variables (from the reconstructed `AHDC::track` bank).
/// Filled once per track BEFORE the track-quality cuts are applied, so the cut
/// thresholds (n_hits, chi2/ndf, ...) can be read off the plots. Only the
/// hits of tracks that pass the cuts contribute to the on-track t0 sample.
struct TrackQualityHists {
    std::unique_ptr<framework::H1<>> n_hits;         ///< hits on the track.
    std::unique_ptr<framework::H1<>> chi2;           ///< sum of squared residuals [mm^2].
    std::unique_ptr<framework::H1<>> chi2ndf;        ///< chi2 / ndf.
    std::unique_ptr<framework::H1<>> sum_residuals;  ///< sum of residuals [mm].
    std::unique_ptr<framework::H1<>> p;              ///< momentum magnitude [MeV].
    std::unique_ptr<framework::H1<>> dEdx;           ///< deposited energy / path [adc/mm].

    void create(const AhdcT0Config& c);  // defined after AhdcT0Config below.

    void fill(int nh, double c2, double c2ndf, double sumres, double mom, double de) const {
        n_hits->fill(static_cast<double>(nh));
        chi2->fill(c2);
        chi2ndf->fill(c2ndf);
        sum_residuals->fill(sumres);
        p->fill(mom);
        dEdx->fill(de);
    }
};

/// Configuration for AhdcT0Alg (reflected to the `[AhdcT0Alg]` TOML section).
struct AhdcT0Config {
    bool   enable_histograms = true;   ///< master switch for booking + filling.
    bool   require_good_wf   = true;   ///< keep only wfType <= 2 (good waveforms).
    double t0_min            = -100.0; ///< timing-axis lower edge [ns].
    double t0_max            = 1000.0; ///< timing-axis upper edge [ns].
    int    t0_bins           = 550;    ///< timing-axis bin count (2 ns/bin default).
    bool   book_global       = true;   ///< also book the summed-over-wires spectra.
    bool   debug_geometry    = false;  ///< one-shot dump of the first event's adc/hits rows.

    // ---- Track quality cuts (gate the on-track sample) ----------------------
    // The quality variables come from the reconstructed AHDC::kftrack bank and
    // are properties of the WHOLE track (number of hits, chi2/ndf, ...). Only
    // the hits of tracks passing every enabled cut contribute to the on-track
    // t0 sample. Each track's variables are ALWAYS histogrammed BEFORE the cuts
    // so the thresholds below can be chosen by inspecting the plots. A cut value
    // of 0 disables that individual cut, so the defaults leave the on-track
    // sample unchanged until thresholds are filled in.
    bool   apply_track_quality = true;   ///< master switch for the cuts below.
    int    track_nhits_min     = 0;      ///< keep tracks with n_hits >= this (0 = off).
    double track_chi2ndf_max   = 0.0;    ///< keep tracks with chi2/ndf <= this (0 = off).
    int    track_ndf_offset    = 5;      ///< ndf = n_hits - this (helix fit: 5 params).

    // ---- Quality-plot axes (used only for the "before cut" diagnostics) -----
    int    q_nhits_max    = 40;        ///< n_hits axis upper edge.
    double q_chi2_max     = 50.0;      ///< chi2 axis upper edge [mm^2].
    double q_chi2ndf_max  = 20.0;      ///< chi2/ndf axis upper edge.
    double q_sumres_max   = 10.0;      ///< sum_residuals axis upper edge [mm].
    double q_p_max        = 400.0;     ///< momentum axis upper edge [MeV].
    double q_dedx_max     = 5000.0;    ///< dEdx axis upper edge [adc/mm].
    int    q_bins         = 200;       ///< bin count for the quality plots.

    static constexpr auto config_fields() {
        using T = AhdcT0Config;
        return std::make_tuple(
            framework::field("enable_histograms",  &T::enable_histograms),
            framework::field("require_good_wf",     &T::require_good_wf),
            framework::field("t0_min",              &T::t0_min),
            framework::field("t0_max",              &T::t0_max),
            framework::field("t0_bins",             &T::t0_bins),
            framework::field("book_global",         &T::book_global),
            framework::field("debug_geometry",      &T::debug_geometry),
            framework::field("apply_track_quality", &T::apply_track_quality),
            framework::field("track_nhits_min",     &T::track_nhits_min),
            framework::field("track_chi2ndf_max",   &T::track_chi2ndf_max),
            framework::field("track_ndf_offset",    &T::track_ndf_offset),
            framework::field("q_nhits_max",         &T::q_nhits_max),
            framework::field("q_chi2_max",          &T::q_chi2_max),
            framework::field("q_chi2ndf_max",       &T::q_chi2ndf_max),
            framework::field("q_sumres_max",        &T::q_sumres_max),
            framework::field("q_p_max",             &T::q_p_max),
            framework::field("q_dedx_max",          &T::q_dedx_max),
            framework::field("q_bins",              &T::q_bins));
    }
};

inline void TrackQualityHists::create(const AhdcT0Config& c) {
    const auto nbins = static_cast<std::size_t>(c.q_bins > 0 ? c.q_bins : 1);
    const auto nh    = static_cast<std::size_t>(c.q_nhits_max > 0 ? c.q_nhits_max : 1);
    n_hits        = std::make_unique<framework::H1<>>(mh::regular(0.0, static_cast<double>(c.q_nhits_max), nh));
    chi2          = std::make_unique<framework::H1<>>(mh::regular(0.0, c.q_chi2_max, nbins));
    chi2ndf       = std::make_unique<framework::H1<>>(mh::regular(0.0, c.q_chi2ndf_max, nbins));
    sum_residuals = std::make_unique<framework::H1<>>(mh::regular(0.0, c.q_sumres_max, nbins));
    p             = std::make_unique<framework::H1<>>(mh::regular(0.0, c.q_p_max, nbins));
    dEdx          = std::make_unique<framework::H1<>>(mh::regular(0.0, c.q_dedx_max, nbins));
}

/**
 * @brief AHDC per-wire t0 extraction algorithm.
 */
class AhdcT0Alg : public framework::Algorithm {
   public:
    using Config = AhdcT0Config;
    using StatusCode = framework::StatusCode;

    explicit AhdcT0Alg(std::string name = "AhdcT0Alg", const Config& config = Config{})
        : Algorithm(std::move(name)), m_config(config) {
        required_services(m_hist);
        // AHDC::adc is the uncalibrated source — required. AHDC::hits carries the
        // trackid used for the on-track sample — optional (absent on files with
        // raw ADC but no calibrated hits; then every hit is "all" only).
        declare_banks("AHDC::adc");
        declare_optional_bank("AHDC::hits");
        // AHDC::kftrack carries the per-track quality variables (n_hits, chi2,
        // ...) used to gate the on-track sample — optional (absent on files
        // without reconstructed tracks; then no track-quality cut is applied).
        declare_optional_bank("AHDC::kftrack");
        declare_config(m_config);
        // Per-wire histograms are dynamic; they are booked in on_initialize().
    }

    [[nodiscard]] auto config() const -> const Config& { return m_config; }

    auto on_initialize() -> StatusCode override {
        if (!m_config.enable_histograms) return StatusCode::SUCCESS();

        const auto nbins = static_cast<std::size_t>(m_config.t0_bins > 0 ? m_config.t0_bins : 1);
        auto& svc = *m_hist;

        for (const auto& g : kAhdcGeom) {
            const int packed = g.superlayer * 10 + g.layer;
            for (int wire = 1; wire <= g.nwires; ++wire) {
                SampleHists& w = m_wires[channel_key(packed, wire)];
                const std::string dir  = fmt::format("SL{}_L{}/wire_{:03d}", g.superlayer, g.layer, wire);
                const std::string chan = fmt::format("SL{} L{} wire {}", g.superlayer, g.layer, wire);
                book_sample(svc, "all",      dir, chan, w.all,      nbins);
                book_sample(svc, "on_track", dir, chan, w.on_track, nbins);
            }
        }

        if (m_config.book_global) {
            book_sample(svc, "all",      "global", "all wires", m_global.all,      nbins);
            book_sample(svc, "on_track", "global", "all wires", m_global.on_track, nbins);
        }

        // Track quality diagnostics: per-track variables from AHDC::kftrack,
        // filled once per track before the quality cuts are applied.
        book_track_quality(svc, m_track_quality);

        Log::info(name(), "Booked {} per-wire channels = {} histograms{}",
                  m_wires.size(), m_wires.size() * 6,
                  m_config.book_global ? " (+6 global)" : "");
        return StatusCode::SUCCESS();
    }

    auto process(framework::Event& event) -> StatusCode override {
        if (!m_config.enable_histograms) return StatusCode::SUCCESS();

        // Read AHDC::adc exactly once. get_bank() returns a 0-row bank when the
        // bank is absent, so this also covers the "not present" case -- calling
        // has_bank() first would extract the bank a second time every event.
        const auto& adc_bank = event.get_bank("AHDC::adc");
        if (adc_bank.getRows() == 0) return StatusCode::SUCCESS();
        bankdefs::AHDC_adc adc;
        adc.bind(adc_bank);

        // Resolve the wfType-column presence once for the whole run (the chain
        // uses one uniform schema) instead of a schema lookup every event.
        int has_wf = m_has_wftype.load(std::memory_order_relaxed);
        if (has_wf < 0) {
            has_wf = const_cast<hipo::bank&>(adc_bank).getSchema().exists("wfType") ? 1 : 0;
            m_has_wftype.store(has_wf, std::memory_order_relaxed);
            if (m_config.require_good_wf && has_wf == 0) {
                m_wftype_missing.store(true, std::memory_order_relaxed);
            }
        }
        const bool apply_wf_cut = m_config.require_good_wf && has_wf == 1;

        // Read the reconstructed KF tracks. Each track's quality variables are
        // histogrammed BEFORE the cuts (so thresholds can be read off the plots),
        // and the trackids that pass every enabled cut are collected. Only the
        // cuts being ACTIVE makes us gate the on-track sample by this set; with
        // the default (no thresholds set) the on-track sample is unchanged.
        const bool track_cuts_active =
            m_config.apply_track_quality &&
            (m_config.track_nhits_min > 0 || m_config.track_chi2ndf_max > 0.0);

        static thread_local std::unordered_set<int> good_tracks;
        good_tracks.clear();
        long n_trk_rows = 0, n_trk_pass = 0;
        const auto& kf_bank = event.get_bank("AHDC::kftrack");
        const int kf_rows = kf_bank.getRows();
        if (kf_rows > 0) {
            bankdefs::AHDC_kftrack kf;
            kf.bind(kf_bank);
            n_trk_rows = kf_rows;
            for (int t = 0; t < kf_rows; ++t) {
                const int    nh     = kf.n_hits(t);
                const double c2     = kf.chi2(t);
                const int    ndf    = std::max(nh - m_config.track_ndf_offset, 1);
                const double c2ndf  = c2 / static_cast<double>(ndf);
                const double sumres = kf.sum_residuals(t);
                const double px = kf.px(t), py = kf.py(t), pz = kf.pz(t);
                const double mom    = std::sqrt(px * px + py * py + pz * pz);
                m_track_quality.fill(nh, c2, c2ndf, sumres, mom, kf.dEdx(t));

                if (!m_config.apply_track_quality || passes_track(nh, c2ndf)) {
                    ++n_trk_pass;
                    good_tracks.insert(kf.trackid(t));
                }
            }
        } else if (track_cuts_active) {
            m_kftrack_missing.store(true, std::memory_order_relaxed);
        }

        // Build the on-track channel set from the calibrated hits. Reuse a
        // per-thread set to avoid a heap allocation on every event; AHDC::hits
        // is read once (0 rows when absent).
        static thread_local std::unordered_set<int> on_track;
        on_track.clear();
        long n_hits = 0, n_hits_trk = 0, n_hits_pass = 0;
        const auto& hits_bank = event.get_bank("AHDC::hits");
        const int hits_rows = hits_bank.getRows();
        if (hits_rows > 0) {
            bankdefs::AHDC_hits hits;
            hits.bind(hits_bank);
            n_hits = hits_rows;
            for (int j = 0; j < hits_rows; ++j) {
                const int trackid = hits.trackid(j);
                if (trackid < 0) continue;
                ++n_hits_trk;

                // Drop the hit when its track failed the quality cuts. Only gate
                // when a cut is actually active AND the KF tracks are present;
                // otherwise the hit stays on-track (trackid>=0) as before.
                if (track_cuts_active && kf_rows > 0 && good_tracks.count(trackid) == 0) continue;

                ++n_hits_pass;
                const int packed = hits.superlayer(j) * 10 + hits.layer(j);
                on_track.insert(channel_key(packed, hits.wire(j)));
            }
        }

        // One-shot geometry dump, fired on the first event that actually has
        // calibrated hits, to validate the adc<->hits key convention
        // (opt-in via [AhdcT0Alg].debug_geometry).
        if (m_config.debug_geometry && hits_rows > 0 && !m_dumped.exchange(true)) {
            dump_geometry(adc, event);
        }

        // Accumulate counters locally and publish once per event, so the hot
        // loop does plain increments instead of an atomic op per hit.
        const int n_adc_rows = adc.rows();
        long n_adc = 0, n_ontrack = 0, n_unmapped = 0;
        for (int i = 0; i < n_adc_rows; ++i) {
            if (apply_wf_cut && adc.wfType(i) > 2) continue;

            const int   key = channel_key(adc.layer(i), adc.component(i));
            const float t   = adc.time(i);
            const float l   = adc.leadingEdgeTime(i);
            const float c   = adc.constantFractionTime(i);

            auto it = m_wires.find(key);
            if (it == m_wires.end()) {
                ++n_unmapped;
                continue;
            }

            ++n_adc;
            it->second.all.fill(t, l, c);
            if (m_config.book_global) m_global.all.fill(t, l, c);

            if (on_track.count(key) != 0) {
                ++n_ontrack;
                it->second.on_track.fill(t, l, c);
                if (m_config.book_global) m_global.on_track.fill(t, l, c);
            }
        }

        m_n_events.fetch_add(1, std::memory_order_relaxed);
        m_n_adc.fetch_add(n_adc, std::memory_order_relaxed);
        m_n_ontrack.fetch_add(n_ontrack, std::memory_order_relaxed);
        m_n_unmapped.fetch_add(n_unmapped, std::memory_order_relaxed);
        m_n_hits_rows.fetch_add(n_hits, std::memory_order_relaxed);
        m_n_hits_ontrack.fetch_add(n_hits_trk, std::memory_order_relaxed);
        m_n_hits_pass.fetch_add(n_hits_pass, std::memory_order_relaxed);
        m_n_trk_rows.fetch_add(n_trk_rows, std::memory_order_relaxed);
        m_n_trk_pass.fetch_add(n_trk_pass, std::memory_order_relaxed);
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        const long n_adc      = m_n_adc.load();
        const long n_trk      = m_n_ontrack.load();
        const long n_unmapped = m_n_unmapped.load();

        Log::info(name(), "=== AHDC t0 extraction summary ===");
        Log::info(name(), "Events with AHDC::adc:   {}", m_n_events.load());
        Log::info(name(), "Good AHDC::adc hits:     {}", n_adc);
        Log::info(name(), "  on track:              {} ({:.1f}%)",
                  n_trk, n_adc > 0 ? 100.0 * static_cast<double>(n_trk) / static_cast<double>(n_adc) : 0.0);
        const long n_trk_hits  = m_n_hits_ontrack.load();
        const long n_pass_hits = m_n_hits_pass.load();
        const long n_trk_rows  = m_n_trk_rows.load();
        const long n_trk_pass  = m_n_trk_pass.load();
        Log::info(name(), "AHDC::hits rows seen:    {} (with trackid>=0: {})",
                  m_n_hits_rows.load(), n_trk_hits);
        Log::info(name(), "AHDC::kftrack rows seen: {}", n_trk_rows);
        if (m_config.apply_track_quality) {
            Log::info(name(), "KF tracks passing quality cuts: {} / {} ({:.1f}%)",
                      n_trk_pass, n_trk_rows,
                      n_trk_rows > 0 ? 100.0 * static_cast<double>(n_trk_pass) / static_cast<double>(n_trk_rows) : 0.0);
            Log::info(name(), "On-track hits kept (track passed): {} / {} ({:.1f}%)",
                      n_pass_hits, n_trk_hits,
                      n_trk_hits > 0 ? 100.0 * static_cast<double>(n_pass_hits) / static_cast<double>(n_trk_hits) : 0.0);
            log_active_cuts();
        } else {
            Log::info(name(), "Track quality cuts: disabled (apply_track_quality=false)");
        }
        if (m_kftrack_missing.load()) {
            Log::warning(name(),
                         "AHDC::kftrack absent on events with track-associated hits — the "
                         "track-quality cut could not be evaluated; those hits were kept as on-track.");
        }
        if (n_unmapped > 0) {
            Log::warning(name(),
                         "Unmapped hits (no geometry slot): {} — check the layer/wire "
                         "convention against kAhdcGeom (off-by-one or extra layer?).",
                         n_unmapped);
        } else {
            Log::info(name(), "Unmapped hits:           0");
        }
        if (m_wftype_missing.load()) {
            Log::warning(name(),
                         "wfType column absent in AHDC::adc — the good-waveform cut "
                         "(require_good_wf) was NOT applied; all waveforms were kept.");
        }
        return StatusCode::SUCCESS();
    }

   private:
    /// One-shot dump of the first event's AHDC::adc / AHDC::hits rows, to
    /// validate the layer/wire encoding (and the adc<->hits join) on a new
    /// data production. Logged once across the whole run.
    void dump_geometry(bankdefs::AHDC_adc& adc, framework::Event& event) {
        Log::info(name(), "[debug_geometry] AHDC::adc rows={}", adc.rows());
        std::set<int> layers;
        for (int i = 0; i < adc.rows(); ++i) layers.insert(adc.layer(i));
        std::string ls;
        for (int v : layers) ls += std::to_string(v) + " ";
        Log::info(name(), "[debug_geometry] distinct adc.layer values: {}", ls);
        const int nshow = std::min(adc.rows(), 15);
        for (int i = 0; i < nshow; ++i) {
            Log::info(name(),
                      "[debug_geometry] adc[{:2d}] layer={:3d} wire(component)={:3d} "
                      "key={:6d} time={:8.2f} let={:8.2f} cft={:8.2f}",
                      i, adc.layer(i), adc.component(i),
                      channel_key(adc.layer(i), adc.component(i)),
                      adc.time(i), adc.leadingEdgeTime(i), adc.constantFractionTime(i));
        }
        if (event.has_bank("AHDC::hits")) {
            bankdefs::AHDC_hits hits;
            hits.bind(event.get_bank("AHDC::hits"));
            Log::info(name(), "[debug_geometry] AHDC::hits rows={}", hits.rows());
            const int hshow = std::min(hits.rows(), 15);
            for (int j = 0; j < hshow; ++j) {
                const int packed = hits.superlayer(j) * 10 + hits.layer(j);
                Log::info(name(),
                          "[debug_geometry] hits[{:2d}] superlayer={} layer={} wire={:3d} "
                          "packed={:2d} key={:6d} trackid={}",
                          j, hits.superlayer(j), hits.layer(j), hits.wire(j),
                          packed, channel_key(packed, hits.wire(j)), hits.trackid(j));
            }
        }
    }

    /// True if a KF track clears every ENABLED quality cut. A threshold of 0
    /// disables that individual cut (see AhdcT0Config). Called only when
    /// m_config.apply_track_quality is set.
    [[nodiscard]] bool passes_track(int n_hits, double chi2ndf) const noexcept {
        if (m_config.track_nhits_min   > 0   && n_hits  < m_config.track_nhits_min)  return false;
        if (m_config.track_chi2ndf_max > 0.0 && chi2ndf > m_config.track_chi2ndf_max) return false;
        return true;
    }

    /// Report which track-quality cuts were actually active (non-zero thresholds).
    void log_active_cuts() const {
        bool any = false;
        if (m_config.track_nhits_min   > 0)   { Log::info(name(), "  cut: track n_hits >= {}", m_config.track_nhits_min); any = true; }
        if (m_config.track_chi2ndf_max > 0.0) { Log::info(name(), "  cut: track chi2/ndf <= {} (ndf = n_hits - {})", m_config.track_chi2ndf_max, m_config.track_ndf_offset); any = true; }
        if (!any) Log::info(name(), "  (no cut thresholds set — inspect /AHDC/quality plots, then set track_nhits_min / track_chi2ndf_max)");
    }

    /// Create + register the per-track quality-variable histograms (from the
    /// reconstructed AHDC::kftrack bank), filled before the cuts.
    void book_track_quality(framework::HistogramService& svc, TrackQualityHists& q) {
        q.create(m_config);
        svc.register_hist("/AHDC/quality/n_hits",
                          "AHDC KF-track n_hits;hits on track;Tracks", *q.n_hits);
        svc.register_hist("/AHDC/quality/chi2",
                          "AHDC KF-track chi2;#chi^{2} [mm^{2}];Tracks", *q.chi2);
        svc.register_hist("/AHDC/quality/chi2ndf",
                          "AHDC KF-track chi2/ndf;#chi^{2}/ndf;Tracks", *q.chi2ndf);
        svc.register_hist("/AHDC/quality/sum_residuals",
                          "AHDC KF-track sum_residuals;#Sigma residuals [mm];Tracks", *q.sum_residuals);
        svc.register_hist("/AHDC/quality/p",
                          "AHDC KF-track momentum;p [MeV];Tracks", *q.p);
        svc.register_hist("/AHDC/quality/dEdx",
                          "AHDC KF-track dE/dx;dE/dx [adc/mm];Tracks", *q.dEdx);
    }

    /// Create + register the three timing histograms of one sample for one
    /// channel (or the global spectra when dir == "global").
    void book_sample(framework::HistogramService& svc, const std::string& sample,
                     const std::string& dir, const std::string& chan,
                     ColumnSet& cs, std::size_t nbins) {
        cs.create(m_config.t0_min, m_config.t0_max, nbins);
        svc.register_hist(
            fmt::format("/AHDC/t0/time/{}/{}", sample, dir),
            fmt::format("AHDC time (max ADC) {} [{}];t [ns];Hits", chan, sample),
            *cs.time);
        svc.register_hist(
            fmt::format("/AHDC/t0/leadingEdgeTime/{}/{}", sample, dir),
            fmt::format("AHDC leadingEdgeTime {} [{}];t [ns];Hits", chan, sample),
            *cs.let);
        svc.register_hist(
            fmt::format("/AHDC/t0/constantFractionTime/{}/{}", sample, dir),
            fmt::format("AHDC constantFractionTime {} [{}];t [ns];Hits", chan, sample),
            *cs.cft);
    }

    Config m_config;

    framework::ServiceRef<framework::HistogramService> m_hist;

    // Per-channel histograms, keyed by channel_key(packed_layer, wire). Built
    // entirely in on_initialize() and never modified afterwards, so concurrent
    // find()/fill() from worker threads is safe (fills use lock-free
    // AtomicStorage; the map is read-only during process()).
    std::unordered_map<int, SampleHists> m_wires;
    SampleHists m_global;

    // Per-track quality diagnostics (AHDC::kftrack). Booked in on_initialize()
    // and read-only during process() (fills use lock-free AtomicStorage).
    TrackQualityHists m_track_quality;

    std::atomic<long> m_n_events{0};
    std::atomic<long> m_n_adc{0};
    std::atomic<long> m_n_ontrack{0};
    std::atomic<long> m_n_unmapped{0};
    std::atomic<long> m_n_hits_rows{0};        ///< total AHDC::hits rows seen.
    std::atomic<long> m_n_hits_ontrack{0};     ///< AHDC::hits rows with trackid>=0.
    std::atomic<long> m_n_hits_pass{0};        ///< on-track hits kept (track passed cuts).
    std::atomic<long> m_n_trk_rows{0};         ///< total AHDC::kftrack rows seen.
    std::atomic<long> m_n_trk_pass{0};         ///< KF tracks passing the quality cuts.
    std::atomic<bool> m_dumped{false};         ///< debug_geometry one-shot guard.
    std::atomic<bool> m_wftype_missing{false};  ///< set if the wfType cut was skipped.
    std::atomic<bool> m_kftrack_missing{false}; ///< set if a track cut couldn't be evaluated.
    std::atomic<int>  m_has_wftype{-1};         ///< wfType column present? -1=unknown,0=no,1=yes.
};

}  // namespace ahdc_t0
