#pragma once

/**
 * @file NTupleWriter.hpp
 * @brief Simplified N-tuple writing with struct-based data and automatic branch registration
 *
 * Provides a convenient wrapper around NTupleService that allows users to define
 * their data as a struct and have branches automatically registered.
 *
 * @see NTupleService
 */

#include <string>
#include <type_traits>
#include <clas12/framework/services/NTupleService.hpp>

namespace clas12::framework {

/**
 * @brief Helper macro to define N-tuple field registration
 *
 * Use this macro inside your data struct to enable automatic branch registration.
 * Fields must be listed in the exact order they are declared.
 *
 * Example:
 * @code
 * struct ElectronData {
 *     double e_p = 0.0;
 *     double e_theta = 0.0;
 *     double e_phi = 0.0;
 *     int sector = 0;
 *
 *     // Register all fields for automatic branch creation
 *     NTUPLE_FIELDS(e_p, e_theta, e_phi, sector)
 * };
 * @endcode
 *
 * @note Maximum of 20 fields supported
 */
#define NTUPLE_FIELDS(...) \
    template <typename NTupleSvc> \
    auto register_branches(NTupleSvc* svc, const std::string& tree_name) -> void { \
        register_branches_impl(svc, tree_name, __VA_ARGS__); \
    } \
    template <typename NTupleSvc, typename T, typename... Ts> \
    auto register_branches_impl(NTupleSvc* svc, const std::string& tree_name, \
                                 T& first, Ts&... rest) -> void { \
        svc->register_branch(tree_name, get_field_name(first), &first); \
        if constexpr (sizeof...(rest) > 0) { \
            register_branches_impl(svc, tree_name, rest...); \
        } \
    } \
    template <typename T> \
    static constexpr auto get_field_name([[maybe_unused]] T& field) -> const char* { \
        return "field"; \
    }

/**
 * @brief Alternative macro with explicit field names
 *
 * Use when you want custom branch names different from variable names.
 *
 * Example:
 * @code
 * struct ElectronData {
 *     double momentum = 0.0;
 *     double theta = 0.0;
 *
 *     NTUPLE_FIELDS_NAMED(
 *         (momentum, "e_p"),
 *         (theta, "e_theta")
 *     )
 * };
 * @endcode
 */
#define NTUPLE_FIELD(var, name) svc->register_branch(tree_name, name, &var)

#define NTUPLE_FIELDS_NAMED(...) \
    template <typename NTupleSvc> \
    auto register_branches(NTupleSvc* svc, const std::string& tree_name) -> void { \
        __VA_ARGS__; \
    }


/**
 * @brief Type-safe N-tuple writer with automatic branch registration
 *
 * Wraps NTupleService to provide a simpler interface for writing event data.
 * Data is defined as a struct with the NTUPLE_FIELDS macro for automatic
 * branch registration.
 *
 * Example usage:
 * @code
 * // Define your data struct
 * struct ElectronNTuple {
 *     double e_p = 0.0;
 *     double e_theta = 0.0;
 *     double e_phi = 0.0;
 *     double e_vz = 0.0;
 *     int sector = 0;
 *
 *     // Simple registration - branches named after variables
 *     template <typename Svc>
 *     auto register_branches(Svc* svc, const std::string& tree) -> void {
 *         svc->register_branch(tree, "e_p", &e_p);
 *         svc->register_branch(tree, "e_theta", &e_theta);
 *         svc->register_branch(tree, "e_phi", &e_phi);
 *         svc->register_branch(tree, "e_vz", &e_vz);
 *         svc->register_branch(tree, "sector", &sector);
 *     }
 * };
 *
 * // In algorithm:
 * class MyAlgorithm : public Algorithm<MyAlgorithm> {
 *     NTupleWriter<ElectronNTuple> m_ntuple;
 *
 *     auto on_initialize() -> StatusCode override {
 *         if (auto* svc = svc<NTupleService>()) {
 *             m_ntuple.init(svc, "electron_analysis");
 *         }
 *         return StatusCode::SUCCESS();
 *     }
 *
 *     auto process(Event& event) -> StatusCode override {
 *         // Fill data
 *         m_ntuple.data().e_p = electron.p();
 *         m_ntuple.data().e_theta = electron.theta();
 *         m_ntuple.data().sector = sector;
 *
 *         // Write entry
 *         m_ntuple.fill();
 *
 *         return StatusCode::SUCCESS();
 *     }
 * };
 * @endcode
 *
 * @tparam T Data struct type (must have register_branches method)
 */
template <typename T>
class NTupleWriter {
public:
    NTupleWriter() = default;

    /**
     * @brief Initialize the writer with a service and tree name
     *
     * @param svc NTupleService pointer
     * @param tree_name Name of the TTree
     * @return bool True if initialization successful
     */
    auto init(NTupleService* svc, const std::string& tree_name) -> bool {
        if (!svc) {
            return false;
        }
        m_service = svc;
        m_tree_name = tree_name;
        m_initialized = true;

        // Register all branches using the struct's register_branches method
        m_data.register_branches(svc, tree_name);

        return true;
    }

    /**
     * @brief Get reference to the data struct for modification
     * @return T& Reference to the data
     */
    auto data() -> T& {
        return m_data;
    }

    /**
     * @brief Get const reference to the data struct
     * @return const T& Const reference to the data
     */
    [[nodiscard]] auto data() const -> const T& {
        return m_data;
    }

    /**
     * @brief Fill the tree with current data
     *
     * Writes the current values in data() to the tree.
     */
    auto fill() -> void {
        if (m_initialized && m_service) {
            m_service->fill(m_tree_name);
        }
    }

    /**
     * @brief Check if the writer is initialized
     */
    [[nodiscard]] auto is_initialized() const -> bool {
        return m_initialized;
    }

    /**
     * @brief Get the tree name
     */
    [[nodiscard]] auto tree_name() const -> const std::string& {
        return m_tree_name;
    }

    /**
     * @brief Get number of entries written
     */
    [[nodiscard]] auto entries() const -> Long64_t {
        return m_service ? m_service->get_entries(m_tree_name) : 0;
    }

    /**
     * @brief Implicit bool conversion to check if initialized
     */
    explicit operator bool() const {
        return m_initialized;
    }

private:
    T m_data{};
    NTupleService* m_service = nullptr;
    std::string m_tree_name;
    bool m_initialized = false;
};


/**
 * @brief Convenience macro to define a simple NTuple struct with registration
 *
 * Creates a struct with the specified fields and automatic branch registration.
 *
 * Example:
 * @code
 * DEFINE_NTUPLE_STRUCT(ElectronNTuple,
 *     double, e_p,
 *     double, e_theta,
 *     double, e_phi,
 *     int, sector
 * );
 *
 * // Use with NTupleWriter:
 * NTupleWriter<ElectronNTuple> m_ntuple;
 * @endcode
 */
#define DEFINE_NTUPLE_STRUCT_1(name, t1, n1) \
    struct name { \
        t1 n1 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_2(name, t1, n1, t2, n2) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_3(name, t1, n1, t2, n2, t3, n3) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; t3 n3 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
            svc->register_branch(tree, #n3, &n3); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_4(name, t1, n1, t2, n2, t3, n3, t4, n4) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; t3 n3 = {}; t4 n4 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
            svc->register_branch(tree, #n3, &n3); \
            svc->register_branch(tree, #n4, &n4); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_5(name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; t3 n3 = {}; t4 n4 = {}; t5 n5 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
            svc->register_branch(tree, #n3, &n3); \
            svc->register_branch(tree, #n4, &n4); \
            svc->register_branch(tree, #n5, &n5); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_6(name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; t3 n3 = {}; t4 n4 = {}; t5 n5 = {}; t6 n6 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
            svc->register_branch(tree, #n3, &n3); \
            svc->register_branch(tree, #n4, &n4); \
            svc->register_branch(tree, #n5, &n5); \
            svc->register_branch(tree, #n6, &n6); \
        } \
    }

#define DEFINE_NTUPLE_STRUCT_7(name, t1, n1, t2, n2, t3, n3, t4, n4, t5, n5, t6, n6, t7, n7) \
    struct name { \
        t1 n1 = {}; t2 n2 = {}; t3 n3 = {}; t4 n4 = {}; t5 n5 = {}; t6 n6 = {}; t7 n7 = {}; \
        template <typename Svc> \
        auto register_branches(Svc* svc, const std::string& tree) -> void { \
            svc->register_branch(tree, #n1, &n1); \
            svc->register_branch(tree, #n2, &n2); \
            svc->register_branch(tree, #n3, &n3); \
            svc->register_branch(tree, #n4, &n4); \
            svc->register_branch(tree, #n5, &n5); \
            svc->register_branch(tree, #n6, &n6); \
            svc->register_branch(tree, #n7, &n7); \
        } \
    }


}  // namespace clas12::framework
