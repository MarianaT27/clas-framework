#pragma once

/**
 * @file NTupleStruct.hpp
 * @brief Struct-based N-Tuple registration with automatic branch creation
 *
 * This file provides utilities for automatically registering N-Tuple branches
 * from struct fields, eliminating repetitive register_branch() calls.
 *
 * Example usage:
 * @code
 * // BEFORE: 5 lines of repetitive registration
 * svc->register_branch("tree", "e_p", &m_e_p);
 * svc->register_branch("tree", "e_theta", &m_e_theta);
 * svc->register_branch("tree", "e_phi", &m_e_phi);
 * svc->register_branch("tree", "e_vz", &m_e_vz);
 * svc->register_branch("tree", "e_chi2pid", &m_e_chi2pid);
 *
 * // AFTER: Struct-based registration
 * struct ElectronNTuple {
 *     double e_p = 0, e_theta = 0, e_phi = 0, e_vz = 0, e_chi2pid = 0;
 * };
 *
 * NTupleWriter<ElectronNTuple> m_ntuple;
 *
 * // In on_initialize():
 * m_ntuple.init(m_ntuple_svc->get(), "electron_kinematics");
 *
 * // In process():
 * m_ntuple.data().e_p = electron.p();
 * m_ntuple.fill();
 * @endcode
 */

#include <mutex>
#include <string>
#include <functional>
#include <vector>

#include <clas12/framework/services/NTupleService.hpp>

namespace clas12::framework {

/**
 * @brief Macro to declare N-Tuple fields with automatic registration
 *
 * Use this macro at the end of your struct to enable automatic branch registration.
 * The macro generates a register_branches() method that registers all listed fields.
 *
 * @param ... List of field names to register as branches
 *
 * Example:
 * @code
 * struct MyNTuple {
 *     double var1 = 0;
 *     double var2 = 0;
 *     int var3 = 0;
 *
 *     NTUPLE_FIELDS(var1, var2, var3)
 * };
 * @endcode
 */
#define NTUPLE_FIELDS(...)                                                              \
    template <typename NTupleSvc>                                                       \
    void register_branches(NTupleSvc* svc, const std::string& tree_name) {              \
        if (!svc) return;                                                               \
        NTUPLE_REGISTER_EACH(svc, tree_name, __VA_ARGS__)                                \
    }

// Helper macros for variadic expansion
#define NTUPLE_REGISTER_EACH(svc, tree, ...) \
    NTUPLE_EXPAND(NTUPLE_REGISTER_IMPL(svc, tree, __VA_ARGS__))

#define NTUPLE_EXPAND(x) x

// Count arguments (up to 20)
#define NTUPLE_NARGS(...) NTUPLE_NARGS_IMPL(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define NTUPLE_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N

// Dispatch to correct implementation based on argument count
#define NTUPLE_REGISTER_IMPL(svc, tree, ...) \
    NTUPLE_CAT(NTUPLE_REGISTER_, NTUPLE_NARGS(__VA_ARGS__))(svc, tree, __VA_ARGS__)

#define NTUPLE_CAT(a, b) NTUPLE_CAT_IMPL(a, b)
#define NTUPLE_CAT_IMPL(a, b) a##b

// Single field registration
#define NTUPLE_REG(svc, tree, field) svc->register_branch(tree, #field, &field);

// Implementations for 1-20 fields
#define NTUPLE_REGISTER_1(svc, tree, f1) NTUPLE_REG(svc, tree, f1)
#define NTUPLE_REGISTER_2(svc, tree, f1, f2) NTUPLE_REG(svc, tree, f1) NTUPLE_REG(svc, tree, f2)
#define NTUPLE_REGISTER_3(svc, tree, f1, f2, f3) NTUPLE_REGISTER_2(svc, tree, f1, f2) NTUPLE_REG(svc, tree, f3)
#define NTUPLE_REGISTER_4(svc, tree, f1, f2, f3, f4) NTUPLE_REGISTER_3(svc, tree, f1, f2, f3) NTUPLE_REG(svc, tree, f4)
#define NTUPLE_REGISTER_5(svc, tree, f1, f2, f3, f4, f5) NTUPLE_REGISTER_4(svc, tree, f1, f2, f3, f4) NTUPLE_REG(svc, tree, f5)
#define NTUPLE_REGISTER_6(svc, tree, f1, f2, f3, f4, f5, f6) NTUPLE_REGISTER_5(svc, tree, f1, f2, f3, f4, f5) NTUPLE_REG(svc, tree, f6)
#define NTUPLE_REGISTER_7(svc, tree, f1, f2, f3, f4, f5, f6, f7) NTUPLE_REGISTER_6(svc, tree, f1, f2, f3, f4, f5, f6) NTUPLE_REG(svc, tree, f7)
#define NTUPLE_REGISTER_8(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8) NTUPLE_REGISTER_7(svc, tree, f1, f2, f3, f4, f5, f6, f7) NTUPLE_REG(svc, tree, f8)
#define NTUPLE_REGISTER_9(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9) NTUPLE_REGISTER_8(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8) NTUPLE_REG(svc, tree, f9)
#define NTUPLE_REGISTER_10(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10) NTUPLE_REGISTER_9(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9) NTUPLE_REG(svc, tree, f10)
#define NTUPLE_REGISTER_11(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11) NTUPLE_REGISTER_10(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10) NTUPLE_REG(svc, tree, f11)
#define NTUPLE_REGISTER_12(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12) NTUPLE_REGISTER_11(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11) NTUPLE_REG(svc, tree, f12)
#define NTUPLE_REGISTER_13(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13) NTUPLE_REGISTER_12(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12) NTUPLE_REG(svc, tree, f13)
#define NTUPLE_REGISTER_14(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) NTUPLE_REGISTER_13(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13) NTUPLE_REG(svc, tree, f14)
#define NTUPLE_REGISTER_15(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15) NTUPLE_REGISTER_14(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) NTUPLE_REG(svc, tree, f15)
#define NTUPLE_REGISTER_16(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16) NTUPLE_REGISTER_15(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15) NTUPLE_REG(svc, tree, f16)
#define NTUPLE_REGISTER_17(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17) NTUPLE_REGISTER_16(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16) NTUPLE_REG(svc, tree, f17)
#define NTUPLE_REGISTER_18(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18) NTUPLE_REGISTER_17(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17) NTUPLE_REG(svc, tree, f18)
#define NTUPLE_REGISTER_19(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19) NTUPLE_REGISTER_18(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18) NTUPLE_REG(svc, tree, f19)
#define NTUPLE_REGISTER_20(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19, f20) NTUPLE_REGISTER_19(svc, tree, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16, f17, f18, f19) NTUPLE_REG(svc, tree, f20)


/**
 * @brief Type trait to check if a type has NTUPLE_FIELDS macro
 */
template <typename T, typename = void>
struct has_ntuple_fields : std::false_type {};

template <typename T>
struct has_ntuple_fields<T, std::void_t<decltype(std::declval<T>().register_branches(
                                std::declval<NTupleService*>(), std::declval<std::string>()))>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_ntuple_fields_v = has_ntuple_fields<T>::value;


/**
 * @brief RAII wrapper for struct-based N-Tuple management
 *
 * NTupleWriter provides convenient management of N-Tuple data structs with:
 * - Automatic branch registration from struct fields
 * - Thread-safe filling
 * - Clean data() accessor for modification
 *
 * The template parameter T must be a struct with the NTUPLE_FIELDS macro.
 *
 * Example:
 * @code
 * struct ElectronData {
 *     double p = 0;
 *     double theta = 0;
 *     double phi = 0;
 *
 *     NTUPLE_FIELDS(p, theta, phi)
 * };
 *
 * class MyAlgorithm {
 *     NTupleWriter<ElectronData> m_electron_nt;
 *
 *     auto on_initialize() -> StatusCode {
 *         m_electron_nt.init(m_ntuple_svc->get(), "electrons");
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto process(Event& event) -> StatusCode {
 *         m_electron_nt.data().p = electron.p();
 *         m_electron_nt.data().theta = electron.theta();
 *         m_electron_nt.data().phi = electron.phi();
 *         m_electron_nt.fill();
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @tparam T Struct type with NTUPLE_FIELDS macro
 */
template <typename T>
class NTupleWriter {
    static_assert(has_ntuple_fields_v<T>,
                  "NTupleWriter requires a struct with NTUPLE_FIELDS macro");

public:
    NTupleWriter() = default;

    /**
     * @brief Construct and initialize with service and tree name
     *
     * @param svc N-Tuple service pointer
     * @param tree_name Name of the TTree
     */
    NTupleWriter(NTupleService* svc, const std::string& tree_name) {
        init(svc, tree_name);
    }

    /**
     * @brief Initialize the writer (register branches)
     *
     * Call this in on_initialize() after services are available.
     *
     * @param svc N-Tuple service pointer
     * @param tree_name Name of the TTree
     */
    auto init(NTupleService* svc, const std::string& tree_name) -> void {
        m_svc = svc;
        m_tree_name = tree_name;
        if (svc) {
            m_data.register_branches(svc, tree_name);
            m_initialized = true;
        }
    }

    /**
     * @brief Check if writer is initialized
     */
    [[nodiscard]] auto is_initialized() const -> bool { return m_initialized && m_svc != nullptr; }

    /**
     * @brief Access the data struct for modification
     *
     * @return T& Reference to the data struct
     */
    auto data() -> T& { return m_data; }

    /**
     * @brief Access the data struct (const)
     */
    [[nodiscard]] auto data() const -> const T& { return m_data; }

    /**
     * @brief Fill the tree with current data (thread-safe via NTupleService)
     */
    auto fill() -> void {
        if (m_svc && m_initialized) {
            m_svc->fill(m_tree_name);
        }
    }

    /**
     * @brief Fill conditionally
     *
     * @param condition Only fill if condition is true
     */
    auto fill_if(bool condition) -> void {
        if (condition) {
            fill();
        }
    }

    /**
     * @brief Get the tree name
     */
    [[nodiscard]] auto tree_name() const -> const std::string& { return m_tree_name; }

    /**
     * @brief Get the underlying service
     */
    [[nodiscard]] auto service() const -> NTupleService* { return m_svc; }

    /**
     * @brief Implicit bool conversion for validity checking
     */
    explicit operator bool() const { return is_initialized(); }

private:
    T m_data{};
    NTupleService* m_svc = nullptr;
    std::string m_tree_name;
    bool m_initialized = false;
};


/**
 * @brief Helper function to create an NTupleWriter
 *
 * @tparam T Struct type with NTUPLE_FIELDS macro
 * @param svc N-Tuple service pointer
 * @param tree_name Name of the TTree
 * @return NTupleWriter<T> Initialized writer
 */
template <typename T>
auto make_ntuple_writer(NTupleService* svc, const std::string& tree_name) -> NTupleWriter<T> {
    return NTupleWriter<T>(svc, tree_name);
}

}  // namespace clas12::framework
