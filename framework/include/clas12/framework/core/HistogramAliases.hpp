#pragma once

/**
 * @file HistogramAliases.hpp
 * @brief Convenience aliases and self-registering wrappers for histograms.
 *
 * Two flavours are provided:
 *
 *   1. Raw aliases — `framework::H1<>`, `H2<>`, `H3<>`. Use these when the
 *      histogram needs custom storage (heap-allocated arrays, maps, etc.) or
 *      when the path/title is dynamic. The user is responsible for calling
 *      `m_hist->register_hist(path, title, h)` themselves.
 *
 *   2. Self-registering wrappers — `framework::Hist1<>`, `Hist2<>`, `Hist3<>`.
 *      Each wrapper bundles the path, title, and binning at the declaration
 *      site. The algorithm registers them with one call:
 *
 *      @code
 *      Hist1<> h_p{"/Electron/p", "p;p [GeV];Counts", 0.0, 11.0, 110};
 *      Hist2<> h_p_vs_theta{"/Electron/p_vs_theta", "p vs #theta;…",
 *                           0.0, 45.0, 90, 0.0, 11.0, 110};
 *
 *      // In the algorithm constructor:
 *      declare_histograms(h_p, h_p_vs_theta);
 *      @endcode
 *
 *      `declare_histograms(...)` queues a post-service hook that walks each
 *      wrapper and calls `HistogramService::register_hist(...)` for it. No
 *      separate `book_histograms()` step is needed — binning, path, and
 *      title all live at the member declaration.
 *
 * Storage defaults to AtomicStorage so concurrent fills from worker threads
 * are lock-free; the default accumulator is CountAccumulator (unweighted).
 * Algorithms that need WeightedAccumulator or non-regular axes should reach
 * for `mh::Histogram` directly.
 */

#include <cstddef>
#include <string>
#include <utility>

#include <mh/mh.hpp>

namespace framework {

template <typename Acc = mh::CountAccumulator>
using H1 = mh::Histogram<Acc, mh::AtomicStorage<Acc>, mh::RegularAxis<>>;

template <typename Acc = mh::CountAccumulator>
using H2 = mh::Histogram<Acc, mh::AtomicStorage<Acc>,
                         mh::RegularAxis<>, mh::RegularAxis<>>;

template <typename Acc = mh::CountAccumulator>
using H3 = mh::Histogram<Acc, mh::AtomicStorage<Acc>,
                         mh::RegularAxis<>, mh::RegularAxis<>, mh::RegularAxis<>>;

// ============================================================================
// Self-registering wrappers
// ============================================================================
//
// Each wrapper carries its destination path + title alongside the histogram
// itself, so a single `declare_histograms(...)` call in the algorithm's
// constructor wires them all into the HistogramService at initialize time.
//
// The wrappers also forward `fill(...)` and the read-only `size()`/`at()`
// accessors so users can treat them like ordinary histograms.

template <typename Acc = mh::CountAccumulator>
class Hist1 {
public:
    Hist1(std::string path, std::string title, double lo, double hi, std::size_t n)
        : m_path(std::move(path)),
          m_title(std::move(title)),
          m_h(mh::regular(lo, hi, n)) {}

    [[nodiscard]] auto path()  const noexcept -> const std::string& { return m_path; }
    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }

    [[nodiscard]] auto histogram()       noexcept ->       H1<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H1<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void { m_h.fill(std::forward<Args>(args)...); }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_path;
    std::string m_title;
    H1<Acc>     m_h;
};

template <typename Acc = mh::CountAccumulator>
class Hist2 {
public:
    Hist2(std::string path, std::string title,
          double lo_x, double hi_x, std::size_t n_x,
          double lo_y, double hi_y, std::size_t n_y)
        : m_path(std::move(path)),
          m_title(std::move(title)),
          m_h(mh::regular(lo_x, hi_x, n_x), mh::regular(lo_y, hi_y, n_y)) {}

    [[nodiscard]] auto path()  const noexcept -> const std::string& { return m_path; }
    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }

    [[nodiscard]] auto histogram()       noexcept ->       H2<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H2<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void { m_h.fill(std::forward<Args>(args)...); }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_path;
    std::string m_title;
    H2<Acc>     m_h;
};

template <typename Acc = mh::CountAccumulator>
class Hist3 {
public:
    Hist3(std::string path, std::string title,
          double lo_x, double hi_x, std::size_t n_x,
          double lo_y, double hi_y, std::size_t n_y,
          double lo_z, double hi_z, std::size_t n_z)
        : m_path(std::move(path)),
          m_title(std::move(title)),
          m_h(mh::regular(lo_x, hi_x, n_x),
              mh::regular(lo_y, hi_y, n_y),
              mh::regular(lo_z, hi_z, n_z)) {}

    [[nodiscard]] auto path()  const noexcept -> const std::string& { return m_path; }
    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }

    [[nodiscard]] auto histogram()       noexcept ->       H3<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H3<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void { m_h.fill(std::forward<Args>(args)...); }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_path;
    std::string m_title;
    H3<Acc>     m_h;
};

// ============================================================================
// Reflection-friendly wrappers: title + binning only, NO path
// ============================================================================
//
// `H1Auto<>/H2Auto<>/H3Auto<>` are like `Hist1<>/Hist2<>/Hist3<>` but carry no
// path. They are meant to be members of a plain-aggregate struct; the ROOT path
// is derived from the MEMBER NAME by `framework::register_fields(...)` (see
// HistogramReflect.hpp), so the path is written exactly once — as the member's
// name — instead of repeated in a hand-written book() method.
//
//     struct StageHists {
//         framework::H1Auto<> p{"Electron Momentum;p [GeV/c];Events", 0, 12, 100};
//         framework::H2Auto<> p_vs_theta{"p vs #theta;#theta;p", 0,45,90, 0,12,100};
//     };
//     // register_fields(svc, "/Electron/before", h) -> "/Electron/before/p", ...
//
// The title still carries the ROOT axis labels — that is the only metadata that
// stays inline, and it lives in the value, so no attribute system is needed.
//
// fill() is a no-op until register_fields() enables the histogram. This lets an
// algorithm call `h.fill(...)` unconditionally in process(): a stage that was
// never registered (e.g. histograms disabled in the config, so register_fields
// was skipped) simply costs nothing — no `if (enable_histograms)` guard needed.

template <typename Acc = mh::CountAccumulator>
class H1Auto {
public:
    H1Auto(std::string title, double lo, double hi, std::size_t n)
        : m_title(std::move(title)), m_h(mh::regular(lo, hi, n)) {}

    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }
    [[nodiscard]] auto histogram()       noexcept ->       H1<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H1<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void {
        if (m_enabled) m_h.fill(std::forward<Args>(args)...);
    }

    /// Enable filling. register_fields() calls this when the histogram is
    /// registered with the service; until then fill() is a no-op.
    auto enable() noexcept -> void { m_enabled = true; }
    [[nodiscard]] auto enabled() const noexcept -> bool { return m_enabled; }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_title;
    H1<Acc>     m_h;
    bool        m_enabled = false;
};

template <typename Acc = mh::CountAccumulator>
class H2Auto {
public:
    H2Auto(std::string title,
           double lo_x, double hi_x, std::size_t n_x,
           double lo_y, double hi_y, std::size_t n_y)
        : m_title(std::move(title)),
          m_h(mh::regular(lo_x, hi_x, n_x), mh::regular(lo_y, hi_y, n_y)) {}

    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }
    [[nodiscard]] auto histogram()       noexcept ->       H2<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H2<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void {
        if (m_enabled) m_h.fill(std::forward<Args>(args)...);
    }

    /// Enable filling. register_fields() calls this when the histogram is
    /// registered with the service; until then fill() is a no-op.
    auto enable() noexcept -> void { m_enabled = true; }
    [[nodiscard]] auto enabled() const noexcept -> bool { return m_enabled; }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_title;
    H2<Acc>     m_h;
    bool        m_enabled = false;
};

template <typename Acc = mh::CountAccumulator>
class H3Auto {
public:
    H3Auto(std::string title,
           double lo_x, double hi_x, std::size_t n_x,
           double lo_y, double hi_y, std::size_t n_y,
           double lo_z, double hi_z, std::size_t n_z)
        : m_title(std::move(title)),
          m_h(mh::regular(lo_x, hi_x, n_x),
              mh::regular(lo_y, hi_y, n_y),
              mh::regular(lo_z, hi_z, n_z)) {}

    [[nodiscard]] auto title() const noexcept -> const std::string& { return m_title; }
    [[nodiscard]] auto histogram()       noexcept ->       H3<Acc>& { return m_h; }
    [[nodiscard]] auto histogram() const noexcept -> const H3<Acc>& { return m_h; }

    template <typename... Args>
    auto fill(Args&&... args) -> void {
        if (m_enabled) m_h.fill(std::forward<Args>(args)...);
    }

    /// Enable filling. register_fields() calls this when the histogram is
    /// registered with the service; until then fill() is a no-op.
    auto enable() noexcept -> void { m_enabled = true; }
    [[nodiscard]] auto enabled() const noexcept -> bool { return m_enabled; }

    [[nodiscard]] auto size() const noexcept { return m_h.size(); }
    [[nodiscard]] auto at(std::size_t i) const { return m_h.at(i); }

private:
    std::string m_title;
    H3<Acc>     m_h;
    bool        m_enabled = false;
};

}  // namespace framework
