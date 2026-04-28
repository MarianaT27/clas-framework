#pragma once

/**
 * @file SectorHistograms.hpp
 * @brief Per-sector histogram handles backed by `mh::Histogram`.
 *
 * Per-sector histograms are pervasive in CLAS12 analyses (the detector has
 * 6 DC sectors). The booking helper here hides the 1..N loop and suffix
 * formatting:
 *
 *     framework::SectorHistogramGroup<6> g(m_hist.get(), "/Tracks/Electrons");
 *     PerSectorHist1D<6> h_nphe = g.add_hist1D("nphe", "nphe;nphe;Tracks",
 *                                              51, 0, 50);
 *     // hot path:
 *     h_nphe.fill(sector, v);    // O(1), null-safe on bad sector
 *
 * Implementation: each `PerSectorHist1D<N>` owns N heap-allocated mh::Histogram
 * instances via std::array<unique_ptr<H1<>>, N>. The heap allocation gives us
 * stable addresses so `SectorHistogramGroup::add_hist1D()` can register each
 * slot with `HistogramService` for ROOT writeout — even though the resulting
 * `PerSectorHist1D<N>` is returned by value and assigned into an algorithm
 * member, the underlying histogram heap address is unchanged.
 */

#include <array>
#include <cstddef>
#include <fmt/format.h>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <clas12/framework/core/HistogramAliases.hpp>
#include <clas12/framework/services/HistogramService.hpp>

#include <mh/mh.hpp>

namespace framework {

/**
 * @brief Array-backed handle to one mh::Histogram (1D) per sector.
 *
 * Sector indices are 1-based (1..NSectors). Out-of-range or unset slots are
 * silent no-ops on `fill()`.
 */
template <int NSectors = 6>
struct PerSectorHist1D {
    std::array<std::unique_ptr<H1<>>, NSectors> slots{};

    [[nodiscard]] auto operator[](int sector) const -> H1<>* {
        if (sector < 1 || sector > NSectors) return nullptr;
        return slots[static_cast<std::size_t>(sector - 1)].get();
    }

    auto fill(int sector, double v) const -> void {
        if (auto* h = (*this)[sector]) h->fill(v);
    }

    auto fill_if(bool condition, int sector, double v) const -> void {
        if (condition) fill(sector, v);
    }

    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }
};

/**
 * @brief Array-backed handle to one mh::Histogram (2D) per sector.
 */
template <int NSectors = 6>
struct PerSectorHist2D {
    std::array<std::unique_ptr<H2<>>, NSectors> slots{};

    [[nodiscard]] auto operator[](int sector) const -> H2<>* {
        if (sector < 1 || sector > NSectors) return nullptr;
        return slots[static_cast<std::size_t>(sector - 1)].get();
    }

    auto fill(int sector, double x, double y) const -> void {
        if (auto* h = (*this)[sector]) h->fill(x, y);
    }

    auto fill_if(bool condition, int sector, double x, double y) const -> void {
        if (condition) fill(sector, x, y);
    }

    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }
};

/**
 * @brief Booking helper for a family of per-sector histograms sharing a base path.
 *
 * Each `add_hist{1,2}D` call:
 *   1. Heap-allocates `NSectors` mh::Histogram instances.
 *   2. Registers each with HistogramService at `{base}/{name}_S{s}` for ROOT
 *      writeout, with title suffixed `" (Sector {s})"`.
 *   3. Returns a `PerSectorHist{1,2}D<NSectors>` whose unique_ptrs the caller
 *      stores; the heap addresses are stable through that move/assign.
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
            const auto idx = static_cast<std::size_t>(s - 1);
            handle.slots[idx] = std::make_unique<H1<>>(
                mh::regular(xmin, xmax, static_cast<std::size_t>(nbins))
            );
            if (m_svc) {
                std::string path = fmt::format("{}/{}_S{}", m_base, name, s);
                std::string ttl  = fmt::format("{} (Sector {})", title, s);
                m_svc->register_hist(std::move(path), std::move(ttl), *handle.slots[idx]);
            }
        }
        return handle;
    }

    auto add_hist2D(std::string_view name, std::string_view title,
                    int nbinsx, double xmin, double xmax,
                    int nbinsy, double ymin, double ymax)
        -> PerSectorHist2D<NSectors> {
        PerSectorHist2D<NSectors> handle{};
        for (int s = 1; s <= NSectors; ++s) {
            const auto idx = static_cast<std::size_t>(s - 1);
            handle.slots[idx] = std::make_unique<H2<>>(
                mh::regular(xmin, xmax, static_cast<std::size_t>(nbinsx)),
                mh::regular(ymin, ymax, static_cast<std::size_t>(nbinsy))
            );
            if (m_svc) {
                std::string path = fmt::format("{}/{}_S{}", m_base, name, s);
                std::string ttl  = fmt::format("{} (Sector {})", title, s);
                m_svc->register_hist(std::move(path), std::move(ttl), *handle.slots[idx]);
            }
        }
        return handle;
    }

    [[nodiscard]] auto base_path() const -> const std::string& { return m_base; }
    [[nodiscard]] static constexpr auto num_sectors() -> int { return NSectors; }

   private:
    HistogramService* m_svc;
    std::string       m_base;
};

}  // namespace framework
