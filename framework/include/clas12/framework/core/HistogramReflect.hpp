#pragma once

/**
 * @file HistogramReflect.hpp
 * @brief Auto-register a struct-of-histograms by reflecting its members.
 *
 * Instead of a hand-written `book()` method that lists one
 * `register_hist(dir + "/name", title, member)` per histogram, declare the
 * histograms as `H1Auto/H2Auto/H3Auto` members of a plain-aggregate struct and
 * call `register_fields(svc, "/dir", hs)`. Boost.PFR walks the struct, and each
 * histogram member is registered at `"/dir/<member_name>"` with its own title —
 * so the path is written exactly once, as the member name.
 *
 *     struct StageHists {
 *         framework::H1Auto<> p{"Electron Momentum;p [GeV/c];Events", 0, 12, 100};
 *         framework::H1Auto<> theta{"Electron #theta;#theta [deg];Events", 0, 45, 90};
 *     };
 *     StageHists h;
 *     framework::register_fields(svc, "/Electron/Kinematics/before_cuts", h);
 *     // -> "/Electron/Kinematics/before_cuts/p", ".../theta"
 *
 * `fill()` is unchanged (the members are still named: `h.p.fill(...)`).
 *
 * Non-histogram members (e.g. a leading `std::string` prefix, or a
 * `std::array<std::unique_ptr<H1<>>,N>` of per-channel histograms with
 * runtime-built names) are skipped via the `AutoHistField` concept, so a struct
 * may freely mix reflected histograms with hand-booked dynamic ones.
 */

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <pfr/core_name.hpp>

#include <clas12/framework/services/HistogramService.hpp>

namespace framework {

/// A reflected member is an auto-registering histogram if it exposes a
/// string-like `title()` and a `histogram()` accessor (H1Auto/H2Auto/H3Auto,
/// and equally the path-carrying Hist1/Hist2/Hist3 wrappers).
template <typename T>
concept AutoHistField = requires(T& t) {
    { t.title() } -> std::convertible_to<std::string_view>;
    t.histogram();
};

/**
 * @brief Register every histogram member of @p hs under
 *        `<dir_prefix>/<member_name>`.
 *
 * @p hs must be a plain aggregate (no user-declared constructor, no base class)
 * so Boost.PFR can reflect its members; give each `H*Auto` member its title and
 * binning via a default member initializer. Members that are not histograms are
 * silently skipped.
 */
template <typename HistStruct>
auto register_fields(HistogramService& svc, std::string_view dir_prefix, HistStruct& hs) -> void {
    static_assert(std::is_aggregate_v<HistStruct>,
                  "register_fields: the histogram struct must be a simple aggregate "
                  "(no user-declared constructor or base class) so Boost.PFR can walk it. "
                  "Give each H1Auto/H2Auto/H3Auto member a default member initializer with "
                  "its title + binning.");
    pfr::for_each_field_with_name(hs, [&](std::string_view name, auto& member) {
        using M = std::remove_reference_t<decltype(member)>;
        if constexpr (AutoHistField<M>) {
            std::string path(dir_prefix);
            if (!path.empty() && path.back() != '/') path.push_back('/');
            path.append(name);
            svc.register_hist(std::move(path), member.title(), member.histogram());
            // Enable the wrapper so its fill() is live. Until a histogram is
            // registered its fill() is a no-op, which lets process() call
            // h.fill(...) unconditionally — a stage that was never registered
            // (histograms disabled) costs nothing. The requires-guard keeps this
            // a no-op for path-carrying wrappers that lack enable().
            if constexpr (requires { member.enable(); }) member.enable();
        }
    });
}

}  // namespace framework
