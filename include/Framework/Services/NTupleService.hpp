#pragma once

#include "Framework/IService.hpp"
#include "TTree.h"
#include "TFile.h"
#include "TBranch.h"
#include <fmt/format.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <mutex>

namespace Framework {

/**
 * @brief Service for managing ROOT TTrees/N-tuples
 *
 * Provides centralized TTree management for event-by-event data output.
 * Similar to ATLAS AthenaRootOutputStream and CMS EDM output.
 *
 * Features:
 * - Automatic TTree creation and management
 * - Branch registration and filling
 * - Integrated with HistogramService (same output file)
 * - Multiple TTrees support
 *
 * Example:
 * @code
 * auto ntuple_svc = service<NTupleService>("NTupleService");
 *
 * // Register branches
 * double electron_p, electron_theta;
 * ntuple_svc->register_branch("events", "electron_p", &electron_p);
 * ntuple_svc->register_branch("events", "electron_theta", &electron_theta);
 *
 * // Fill in event loop
 * electron_p = e.p();
 * electron_theta = e.theta();
 * ntuple_svc->fill("events");
 * @endcode
 */
class NTupleService : public Service {
public:
    explicit NTupleService(std::string name = "NTupleService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        fmt::print("NTupleService: Initialized\n");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        if (!m_output_file.empty()) {
            write_trees();
        }
        fmt::print("NTupleService: Finalized\n");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set the output ROOT file
     *
     * Note: Can be the same file as HistogramService for convenience
     */
    auto set_output_file(const std::string& filename) -> void {
        m_output_file = filename;
    }

    /**
     * @brief Get the output file path
     */
    auto get_output_file() const -> const std::string& {
        return m_output_file;
    }

    /**
     * @brief Create a new TTree
     *
     * @param name Tree name
     * @param title Tree title
     * @param auto_save Auto-save every N entries (0 = at end only)
     */
    auto create_tree(const std::string& name, const std::string& title = "",
                     int auto_save = 0) -> TTree* {
        if (m_trees.find(name) != m_trees.end()) {
            fmt::print(stderr, "WARNING: Tree '{}' already exists\n", name);
            return m_trees[name].get();
        }

        std::string tree_title = title.empty() ? name : title;
        auto tree = std::make_unique<TTree>(name.c_str(), tree_title.c_str());
        tree->SetDirectory(nullptr);  // Don't let ROOT own it

        // Set auto-flush to write baskets periodically (every 10MB)
        tree->SetAutoFlush(-10000000);  // Negative = bytes, positive = entries

        if (auto_save > 0) {
            tree->SetAutoSave(auto_save);
        }

        TTree* ptr = tree.get();
        m_trees[name] = std::move(tree);
        return ptr;
    }

    /**
     * @brief Get an existing TTree
     */
    auto get_tree(const std::string& name) -> TTree* {
        auto it = m_trees.find(name);
        return (it != m_trees.end()) ? it->second.get() : nullptr;
    }

    /**
     * @brief Register a branch (creates tree if needed)
     *
     * @param tree_name Name of the tree
     * @param branch_name Name of the branch
     * @param address Pointer to variable
     * @param leaf_list Leaf specification (e.g., "var/D" for double)
     */
    template<typename T>
    auto register_branch(const std::string& tree_name,
                        const std::string& branch_name,
                        T* address,
                        const std::string& leaf_list = "") -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        // Get or create tree
        TTree* tree = get_tree(tree_name);
        if (!tree) {
            tree = create_tree(tree_name);
        }

        // Register branch with basket size limit to prevent 1GB buffer overflow
        TBranch* branch = nullptr;
        if (leaf_list.empty()) {
            branch = tree->Branch(branch_name.c_str(), address);
        } else {
            branch = tree->Branch(branch_name.c_str(), address, leaf_list.c_str());
        }

        // Set basket size to 16KB per branch for safe multi-threaded operation
        if (branch) {
            branch->SetBasketSize(16000);
        }
    }

    /**
     * @brief Register a vector branch
     */
    template<typename T>
    auto register_vector_branch(const std::string& tree_name,
                               const std::string& branch_name,
                               std::vector<T>* address) -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (!tree) {
            tree = create_tree(tree_name);
        }

        // Register branch and set basket size
        TBranch* branch = tree->Branch(branch_name.c_str(), address);
        if (branch) {
            branch->SetBasketSize(16000);
        }
    }

    /**
     * @brief Fill a tree (thread-safe)
     */
    auto fill(const std::string& tree_name) -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (tree) {
            tree->Fill();
        } else {
            fmt::print(stderr, "WARNING: Tree '{}' not found\n", tree_name);
        }
    }

    /**
     * @brief Get number of entries in a tree
     */
    auto get_entries(const std::string& tree_name) const -> Long64_t {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);
        auto it = m_trees.find(tree_name);
        return (it != m_trees.end()) ? it->second->GetEntries() : 0;
    }

    /**
     * @brief Write all trees to output file (thread-safe)
     */
    auto write_trees() -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_output_file.empty()) {
            fmt::print(stderr, "WARNING: No output file set for TTrees\n");
            return;
        }

        if (m_trees.empty()) {
            fmt::print("NTupleService: No TTrees to write\n");
            return;
        }

        fmt::print("\nWriting {} TTrees to: {}\n", m_trees.size(), m_output_file);

        // Open file in UPDATE mode to preserve histograms if they exist
        // Use compression level 9 to reduce file size
        TFile file(m_output_file.c_str(), "UPDATE", "", 9);

        if (file.IsZombie() || !file.IsOpen()) {
            fmt::print(stderr, "ERROR: Failed to open output file: {}\n", m_output_file);
            return;
        }

        // Write all trees
        for (auto& [name, tree] : m_trees) {
            Long64_t entries = tree->GetEntries();

            // Flush baskets to disk before writing to avoid 1GB buffer overflow
            tree->FlushBaskets();

            // Write the tree
            tree->Write("", TObject::kOverwrite);

            fmt::print("  ✓ Wrote tree '{}' ({} entries)\n", name, entries);
        }

        file.Close();
        fmt::print("✓ Successfully wrote {} TTrees\n", m_trees.size());
    }

    /**
     * @brief Print tree summary (thread-safe)
     */
    auto print_summary() const -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        fmt::print("\nN-tuple Summary:\n");
        fmt::print("  Output file: {}\n", m_output_file);
        fmt::print("  Total trees: {}\n", m_trees.size());

        if (!m_trees.empty()) {
            fmt::print("\n  Tree Details:\n");
            for (const auto& [name, tree] : m_trees) {
                Long64_t entries = tree->GetEntries();
                Int_t branches = tree->GetNbranches();
                fmt::print("    {:<20} : {} entries, {} branches\n",
                          name, entries, branches);
            }
        }
        fmt::print("\n");
    }

    /**
     * @brief Get all tree names
     */
    auto get_tree_names() const -> std::vector<std::string> {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);
        std::vector<std::string> names;
        for (const auto& [name, tree] : m_trees) {
            names.push_back(name);
        }
        return names;
    }

    /**
     * @brief Set basket size for optimization (thread-safe)
     */
    auto set_basket_size(const std::string& tree_name, int size) -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);
        TTree* tree = get_tree(tree_name);
        if (tree) {
            tree->SetBasketSize("*", size);
        }
    }

private:
    std::string m_output_file;
    std::map<std::string, std::unique_ptr<TTree>> m_trees;
    mutable std::mutex m_tree_mutex_;
};

/**
 * @brief RAII helper for automatic tree filling
 *
 * Usage:
 * @code
 * {
 *     TreeFiller filler(ntuple_svc, "events");
 *     // Set branch variables...
 *     electron_p = e.p();
 *     electron_theta = e.theta();
 * } // Automatically fills tree on scope exit
 * @endcode
 */
class TreeFiller {
public:
    TreeFiller(NTupleService* service, const std::string& tree_name)
        : m_service(service), m_tree_name(tree_name) {}

    ~TreeFiller() {
        if (m_service) {
            m_service->fill(m_tree_name);
        }
    }

    // Non-copyable, non-movable
    TreeFiller(const TreeFiller&) = delete;
    TreeFiller& operator=(const TreeFiller&) = delete;
    TreeFiller(TreeFiller&&) = delete;
    TreeFiller& operator=(TreeFiller&&) = delete;

private:
    NTupleService* m_service;
    std::string m_tree_name;
};

}  // namespace Framework
