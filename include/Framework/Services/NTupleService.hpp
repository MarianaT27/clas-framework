#pragma once

#include <fmt/format.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"

namespace Framework {

// Forward declaration
class LoggingService;

/**
 * @brief Service for managing ROOT TTrees/N-tuples
 *
 * Provides centralized TTree management for event-by-event data output.
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
        if (m_incremental_enabled && !m_output_file.empty()) {
            m_output_file_ptr = std::make_unique<TFile>(m_output_file.c_str(), "RECREATE");
            if (m_output_file_ptr->IsZombie() || !m_output_file_ptr->IsOpen()) {
                if (m_log) m_log->error(name(), "Failed to open file for incremental writing: {}", m_output_file);
                return StatusCode::FAILURE();
            }
            if (m_log) m_log->info(name(), "Opened output file for incremental writing: {}", m_output_file);
        }
        if (m_log) m_log->info(name(), "Initialized");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        // Write TTrees first while any attached file is still open so trees that
        // reference the file (via SetDirectory) can safely access their data.
        if (!m_output_file.empty()) write_trees();

        if (m_output_file_ptr && m_output_file_ptr->IsOpen()) {
            m_output_file_ptr->Close();
            m_output_file_ptr.reset();
        }
        if (m_log) m_log->info(name(), "Finalized");

        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set logging service for structured output
     */
    auto set_logger(LoggingService* log) -> void { m_log = log; }

    /**
     * @brief Set the output ROOT file
     *
     * Note: Can be the same file as HistogramService for convenience
     */
    auto set_output_file(const std::string& filename) -> void { m_output_file = filename; }

    /**
     * @brief Get the output file path
     */
    auto get_output_file() const -> const std::string& { return m_output_file; }

    /**
     * @brief Create a new TTree
     *
     * @param name Tree name
     * @param title Tree title
     * @param auto_save Auto-save every N entries (0 = at end only)
     */
    auto create_tree(const std::string& name, const std::string& title = "", int auto_save = 0) -> TTree* {
        if (m_trees.find(name) != m_trees.end()) {
            fmt::print(stderr, "WARNING: Tree '{}' already exists\n", name);
            return m_trees[name].get();
        }

        std::string tree_title = title.empty() ? name : title;
        auto tree = std::make_unique<TTree>(name.c_str(), tree_title.c_str());

        // If incremental writing is enabled, attach to file immediately
        if (m_incremental_enabled && m_output_file_ptr && m_output_file_ptr->IsOpen()) {
            m_output_file_ptr->cd();
            tree->SetDirectory(m_output_file_ptr.get());
            tree->SetAutoSave(100000000);  // Auto-save every 100MB
        } else {
            tree->SetDirectory(nullptr);  // Don't let ROOT own it
        }

        tree->SetAutoFlush(-100000); // Flush every 100KB
        tree->SetCircular(500000000LL); // Keep max 500MB of baskets

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
    template <typename T>
    auto register_branch(const std::string& tree_name, const std::string& branch_name, T* address, const std::string& leaf_list = "") -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (!tree) tree = create_tree(tree_name);

        TBranch* branch = nullptr;
        if (leaf_list.empty()) {
            branch = tree->Branch(branch_name.c_str(), address);
        } else {
            branch = tree->Branch(branch_name.c_str(), address, leaf_list.c_str());
        }

        if (branch) branch->SetBasketSize(8000);
    }

    /**
     * @brief Register a vector branch
     */
    template <typename T>
    auto register_vector_branch(const std::string& tree_name, const std::string& branch_name, std::vector<T>* address) -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (!tree) tree = create_tree(tree_name);
        TBranch* branch = tree->Branch(branch_name.c_str(), address);
        if (branch) branch->SetBasketSize(8000);
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
     * This is REQUIRED for very large datasets to avoid 1GB buffer overflow.
     */
    auto enable_incremental_writing(int autosave_bytes = 100000000) -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_output_file.empty()) {
            if (m_log) m_log->warning(name(), "Cannot enable incremental writing: no output file set");
            return;
        }

        m_output_file_ptr = std::make_unique<TFile>(m_output_file.c_str(), "RECREATE", "", 9);

        if (m_output_file_ptr->IsZombie() || !m_output_file_ptr->IsOpen()) {
            if (m_log) m_log->error(name(), "Failed to open file for incremental writing: {}", m_output_file);
            m_output_file_ptr.reset();
            return;
        }

        for (auto& [tree_name, tree] : m_trees) {
            m_output_file_ptr->cd();
            tree->SetDirectory(m_output_file_ptr.get());
            tree->SetAutoSave(autosave_bytes);  // Auto-save every 100MB
        }

        m_incremental_enabled = true;
        if (m_log) m_log->info(name(), "Enabled incremental writing to: {} (AutoSave every {} bytes)", m_output_file, autosave_bytes);
    }

    /**
     * @brief Write all trees to output file (thread-safe)
     */
    auto write_trees() -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_output_file.empty()) {
            if (m_log) m_log->warning(name(), "No output file set for TTrees");
            return;
        }

        if (m_trees.empty()) {
            if (m_log) m_log->info(name(), "No TTrees to write");
            return;
        }

        if (m_log) m_log->info(name(), "Writing {} TTrees to: {}", m_trees.size(), m_output_file);

        TFile* file_ptr = nullptr;

        if (m_log) m_log->info(name(), "Using existing open file for writing TTrees");
        file_ptr = m_output_file_ptr.get();
        file_ptr->cd();

        for (auto& [n, tree] : m_trees) {
            if (m_log) m_log->info(name(), "Writing tree '{}'", n);
            Long64_t entries = tree->GetEntries();
            if (m_log) m_log->info(this->name(), "  Wrote tree '{}' ({} entries)", n, entries);
            tree->FlushBaskets();
            file_ptr->cd();
            tree->Write();
            tree->SetDirectory(nullptr);
        }

        
        file_ptr->Write();
        file_ptr->Close();

        if (m_log) m_log->info(this->name(), "Successfully wrote {} TTrees", m_trees.size());

        m_output_file_ptr->Close();
        if (m_log) m_log->info(name(), "Closed output file: {}", m_output_file);

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
                fmt::print("    {:<20} : {} entries, {} branches\n", name, entries, branches);
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
    bool m_incremental_enabled = false;        // Track if incremental writing is active
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
