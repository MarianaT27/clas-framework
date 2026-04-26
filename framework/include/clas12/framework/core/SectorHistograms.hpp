#pragma once

/**
 * @file SectorHistograms.hpp
 * @brief Per-sector histogram handles and group booking helper.
 *
 * Per-sector histograms are pervasive in CLAS12 analyses (CLAS12 has 6 DC
 * sectors). The older pattern was:
 *
 *     std::map<int, ThreadLocalHistogram1D*> h_nphe;
 *     for (int s = 1; s <= 6; ++s)
 *         h_nphe[s] = svc->book_hist1D(base + "_S" + to_string(s), ...);
 *     // fill:
 *     h_nphe[sector]->fill(v);   // map lookup + implicit null deref
 *
 * The types in this header replace that with a flat array-backed handle and
 * a booking group that hides the 1..N loop and suffix formatting:
 *
 *     SectorHistogramGroup<6> grp(svc, "/Tracks/Electrons");
 *     PerSectorHist1D<6> h_nphe = grp.add_hist1D("nphe", "nphe;nphe;Tracks",
 *                                                51, 0, 50);
 *     // fill:
 *     h_nphe.fill(sector, v);    // O(1), null-safe
 */

#include <array>
#include <fmt/format.h>
#include <map>
#include <string>
#include <string_view>

#include <clas12/framework/core/ThreadLocalHistogram.hpp>
#include <clas12/framework/services/HistogramService.hpp>

namespace clas12::framework {

/**
 * @brief Array-backed handle to one ThreadLocalHistogram1D per sector.
 *
 * Sector indices are 1-based (1..NSectors). Out-of-range or null-slot
 * accesses are silently no-ops, matching the Hist1D wrapper semantics.
 */
template <int NSectors = 6>
struct PerSectorHist1D {
    std::array<ThreadLocalHistogram1D*, NSectors> slots{};

    [[nodiscard]] auto operator[](int sector) const -> ThreadLocalHistogram1D* {
        if (sector < 1 || sector > NSectors) return nullptr;
        return slots[static_cast<std::size_t>(sector - 1)];
    }

    auto fill(int sector, double v, double w = 1.0) const -> void {
        if (auto* h = (*this)[sector]) h->fill(v, w);
    }

    auto fill_if(bool condition, int sector, double v, double w = 1.0) const -> void {
        if (condition) fill(sector, v, w);
    }

    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }
};

/**
 * @brief Array-backed handle to one ThreadLocalHistogram2D per sector.
 */
template <int NSectors = 6>
struct PerSectorHist2D {
    std::array<ThreadLocalHistogram2D*, NSectors> slots{};

    [[nodiscard]] auto operator[](int sector) const -> ThreadLocalHistogram2D* {
        if (sector < 1 || sector > NSectors) return nullptr;
        return slots[static_cast<std::size_t>(sector - 1)];
    }

    auto fill(int sector, double x, double y, double w = 1.0) const -> void {
        if (auto* h = (*this)[sector]) h->fill(x, y, w);
    }

    auto fill_if(bool condition, int sector, double x, double y, double w = 1.0) const -> void {
        if (condition) fill(sector, x, y, w);
    }

    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }
};

/**
 * @brief Booking helper for a family of per-sector histograms sharing a base path.
 *
 * Each `add_hist{1,2}D` call books NSectors atomic histograms at
 * `{base}/{name}_S{s}` with titles suffixed `" (Sector {s})"`, and returns a
 * `PerSectorHist{1,2}D` handle the caller stores for fast fills.
 *
 * Example:
 * @code
 * SectorHistogramGroup<6> g(m_hist, "/Tracks/Electrons");
 * auto h_nphe      = g.add_hist1D("nphe", "Cherenkov nphe;nphe;Tracks", 51, 0, 50);
 * auto h_pcal_ecal = g.add_hist2D("EPCal_vs_ECal",
 *                                 "E_{PCal} vs E_{Cal};E_{PCal} [GeV];E_{Cal} [GeV]",
 *                                 200, 0, 0.6, 200, 0, 0.6);
 * // hot path:
 * h_nphe.fill(sector, nphe);
 * h_pcal_ecal.fill(sector, e_pcal, e_cal);
 * @endcode
 */
template <int NSectors = 6>
class SectorHistogramGroup {
   public:
    SectorHistogramGroup(HistogramService* svc, std::string base_path)
        : m_svc(svc), m_base(std::move(base_path)) {}

    auto add_hist1D(std::string_view name, std::string_view title,
                    int nbins, double xmin, double xmax)
        -> PerSectorHist1D<NSectors> {
        PerSectorHist1D<NSectors> handle{};
        for (int s = 1; s <= NSectors; ++s) {
            std::string path  = fmt::format("{}/{}_S{}", m_base, name, s);
            std::string ttl   = fmt::format("{} (Sector {})", title, s);
            handle.slots[static_cast<std::size_t>(s - 1)] =
                m_svc ? m_svc->book_hist1D(path, ttl, nbins, xmin, xmax) : nullptr;
        }
        m_hist1d.emplace(std::string{name}, handle);
        return handle;
    }

    auto add_hist2D(std::string_view name, std::string_view title,
                    int nbinsx, double xmin, double xmax,
                    int nbinsy, double ymin, double ymax)
        -> PerSectorHist2D<NSectors> {
        PerSectorHist2D<NSectors> handle{};
        for (int s = 1; s <= NSectors; ++s) {
            std::string path = fmt::format("{}/{}_S{}", m_base, name, s);
            std::string ttl  = fmt::format("{} (Sector {})", title, s);
            handle.slots[static_cast<std::size_t>(s - 1)] =
                m_svc ? m_svc->book_hist2D(path, ttl,
                                           nbinsx, xmin, xmax,
                                           nbinsy, ymin, ymax)
                      : nullptr;
        }
        m_hist2d.emplace(std::string{name}, handle);
        return handle;
    }

    [[nodiscard]] auto hist1d(std::string_view name) const -> PerSectorHist1D<NSectors> {
        auto it = m_hist1d.find(std::string{name});
        return it == m_hist1d.end() ? PerSectorHist1D<NSectors>{} : it->second;
    }

    [[nodiscard]] auto hist2d(std::string_view name) const -> PerSectorHist2D<NSectors> {
        auto it = m_hist2d.find(std::string{name});
        return it == m_hist2d.end() ? PerSectorHist2D<NSectors>{} : it->second;
    }

    [[nodiscard]] auto base_path() const -> const std::string& { return m_base; }
    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }

   private:
    HistogramService* m_svc;
    std::string       m_base;
    std::map<std::string, PerSectorHist1D<NSectors>> m_hist1d;
    std::map<std::string, PerSectorHist2D<NSectors>> m_hist2d;
};

}  // namespace clas12::framework
