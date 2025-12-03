#pragma once

#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
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

// Forward declaration
class LoggingService;

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
        if (m_log) {
            m_log->info(name(), "Initialized");
        }
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        if (!m_output_file.empty()) {
            write_trees();
        }

        // Close incremental file if open
        if (m_output_file_ptr && m_output_file_ptr->IsOpen()) {
            m_output_file_ptr->Close();
            m_output_file_ptr.reset();
        }

        if (m_log) {
            m_log->info(name(), "Finalized");
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set logging service for structured output
     */
    auto set_logger(LoggingService* log) -> void {
        m_log = log;
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

        // Aggressive settings to prevent 1GB TBufferFile overflow with large datasets
        // Flush every 100KB (not 10MB) to minimize basket accumulation
        tree->SetAutoFlush(-100000);  // Flush every 100KB

        // Limit tree size in memory to 100MB before flushing
        tree->SetMaxTreeSize(100000000LL);  // 100MB

        // Use circular buffer to limit baskets kept in memory
        tree->SetCircular(500000000LL);  // Keep max 500MB of baskets

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

        // Set basket size to 8KB per branch (reduced for very large datasets)
        if (branch) {
            branch->SetBasketSize(8000);
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
            branch->SetBasketSize(8000);  // 8KB for very large datasets
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
     * @brief Enable incremental writing for very large datasets
     *
     * Call this after setting output file but before filling trees.
     * Opens the file and associates trees with it for auto-saving.
     */
    auto enable_incremental_writing() -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_output_file.empty()) {
            if (m_log) {
                m_log->warning(name(), "Cannot enable incremental writing: no output file set");
            }
            return;
        }

        // Open file in UPDATE mode
        m_output_file_ptr = std::make_unique<TFile>(m_output_file.c_str(), "RECREATE", "", 9);

        if (m_output_file_ptr->IsZombie() || !m_output_file_ptr->IsOpen()) {
            if (m_log) {
                m_log->error(name(), "Failed to open file for incremental writing: {}", m_output_file);
            }
            m_output_file_ptr.reset();
            return;
        }

        if (m_log) {
            m_log->info(name(), "Enabled incremental writing to: {}", m_output_file);
        }
    }

    /**
     * @brief Write all trees to output file (thread-safe)
     */
    auto write_trees() -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_output_file.empty()) {
            if (m_log) {
                m_log->warning(name(), "No output file set for TTrees");
            }
            return;
        }

        if (m_trees.empty()) {
            if (m_log) {
                m_log->info(name(), "No TTrees to write");
            }
            return;
        }

        if (m_log) {
            m_log->info(name(), "Writing {} TTrees to: {}", m_trees.size(), m_output_file);
        }

        TFile* file_ptr = nullptr;
        bool need_close = false;

        // If incremental writing is enabled, use the existing file
        if (m_output_file_ptr && m_output_file_ptr->IsOpen()) {
            file_ptr = m_output_file_ptr.get();
            file_ptr->cd();
        } else {
            // Open file in UPDATE mode to preserve histograms if they exist
            file_ptr = new TFile(m_output_file.c_str(), "UPDATE", "", 9);
            need_close = true;

            if (file_ptr->IsZombie() || !file_ptr->IsOpen()) {
                if (m_log) {
                    m_log->error(name(), "Failed to open output file: {}", m_output_file);
                }
                delete file_ptr;
                return;
            }
        }

        // Write all trees
        for (auto& [name, tree] : m_trees) {
            Long64_t entries = tree->GetEntries();

            // Critical: Flush all baskets before writing
            tree->FlushBaskets();

            // Write the tree with explicit flush
            file_ptr->cd();
            tree->Write("", TObject::kOverwrite);

            if (m_log) {
                m_log->info(this->name(), "  Wrote tree '{}' ({} entries)", name, entries);
            }
        }

        if (need_close) {
            file_ptr->Close();
            delete file_ptr;
        } else {
            // Just write to disk without closing
            file_ptr->Write();
        }

        if (m_log) {
            m_log->info(this->name(), "Successfully wrote {} TTrees", m_trees.size());
        }
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
    LoggingService* m_log = nullptr;
    std::unique_ptr<TFile> m_output_file_ptr;  // For incremental writing
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
