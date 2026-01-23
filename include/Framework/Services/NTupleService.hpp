#pragma once

#include <fmt/format.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "Framework/IService.hpp"
#include "Framework/Services/LoggingService.hpp"
#include "Framework/Services/OutputFileService.hpp"
#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"

namespace Framework {

// Forward declaration
class OutputFileService;

/**
 * @brief Service for managing ROOT TTrees/N-tuples
 *
 * Provides centralized TTree management for event-by-event data output.
 * Uses OutputFileService for all file operations.
 *
 * Features:
 * - Automatic TTree creation and management
 * - Branch registration and filling
 * - Uses OutputFileService for centralized file management
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
    /// Static service name for compile-time access
    static constexpr std::string_view service_name() { return "NTupleService"; }

    explicit NTupleService(std::string name = "NTupleService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        if (!m_output_svc) {
            Log::error(name(), "OutputFileService not set! Call set_output_file_service() before initialize()");
            return StatusCode::FAILURE();
        }

        if (!m_output_svc->is_file_open()) {
            Log::error(name(), "OutputFileService file is not open");
            return StatusCode::FAILURE();
        }

        Log::info(name(), "Initialized with OutputFileService");
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        Log::info(name(), "Finalizing NTupleService, writing {} trees", m_trees.size());
        write_trees();
        Log::info(name(), "Finalized");
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set the OutputFileService (REQUIRED)
     * Must be called before initialize()
     */
    auto set_output_file_service(OutputFileService* svc) -> void {
        m_output_svc = svc;
    }

    /**
     * @brief Get the output file path from OutputFileService
     */
    auto get_output_file() const -> std::string {
        return m_output_svc ? m_output_svc->get_output_file() : "";
    }

    /**
     * @brief Create a new TTree
     *
     * @param name Tree name
     * @param title Tree title
     * @param directory Optional directory path in the file
     */
    auto create_tree(const std::string& name, const std::string& title = "", const std::string& directory = "") -> TTree* {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        if (m_trees.find(name) != m_trees.end()) {
            Log::warning(this->name(), "Tree '{}' already exists", name);
            return m_trees[name].get();
        }

        if (!m_output_svc || !m_output_svc->is_file_open()) {
            Log::error(this->name(), "OutputFileService not available for tree creation");
            return nullptr;
        }

        // Navigate to directory if specified
        {
            std::lock_guard<std::mutex> file_lock(m_output_svc->mutex());
            TFile* file = m_output_svc->get_file();
            if (!file) return nullptr;
            
            if (!directory.empty()) {
                // Create directory path if needed
                m_output_svc->cd(directory);
            } else {
                file->cd();
            }
        }

        std::string tree_title = title.empty() ? name : title;
        auto tree = std::make_unique<TTree>(name.c_str(), tree_title.c_str());

        // Attach to file for incremental writing to avoid 1GB buffer limit
        // Tree is automatically associated with current directory (the file)
        // We'll explicitly detach before file closes to avoid segfault
        tree->SetAutoFlush(-30000000);  // Flush every 30MB
        tree->SetAutoSave(-300000000);   // AutoSave every 300MB

        TTree* ptr = tree.get();
        m_trees[name] = std::move(tree);
        m_tree_directories[name] = directory;
        
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
        std::unique_lock<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (!tree) {
            // Release lock to create tree (which acquires its own lock)
            lock.unlock();
            tree = create_tree(tree_name);
            lock.lock();
            if (!tree) return;
        }

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
        std::unique_lock<std::mutex> lock(m_tree_mutex_);

        TTree* tree = get_tree(tree_name);
        if (!tree) {
            lock.unlock();
            tree = create_tree(tree_name);
            lock.lock();
            if (!tree) return;
        }

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
            Log::warning(this->name(), "Tree '{}' not found", tree_name);
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

        if (!m_output_svc || !m_output_svc->is_file_open()) {
            Log::warning(name(), "OutputFileService not available for writing TTrees");
            return;
        }

        if (m_trees.empty()) {
            Log::info(name(), "No TTrees to write");
            return;
        }

        Log::info(name(), "Writing {} TTrees via OutputFileService", m_trees.size());

        std::lock_guard<std::mutex> file_lock(m_output_svc->mutex());
        TFile* file = m_output_svc->get_file();
        if (!file) return;

        for (auto& [n, tree] : m_trees) {
            // Navigate to the tree's directory
            const std::string& dir = m_tree_directories[n];
            if (!dir.empty()) {
                file->cd(dir.c_str());
            } else {
                file->cd();
            }

            Long64_t entries = tree->GetEntries();

            // Flush any remaining baskets and write
            tree->FlushBaskets();
            tree->Write("", TObject::kOverwrite);

            // IMPORTANT: Detach from directory to prevent segfault when file closes
            tree->SetDirectory(nullptr);

            Log::info(this->name(), "  Wrote tree '{}' ({} entries)", n, entries);
        }

        Log::info(this->name(), "Successfully wrote {} TTrees", m_trees.size());
    }

    /**
     * @brief Print tree summary (thread-safe)
     */
    auto print_summary() const -> void {
        std::lock_guard<std::mutex> lock(m_tree_mutex_);

        fmt::print("\nN-tuple Summary:\n");
        fmt::print("  Output file: {}\n", get_output_file());
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
    std::map<std::string, std::unique_ptr<TTree>> m_trees;
    std::map<std::string, std::string> m_tree_directories;  // Track directory for each tree
    mutable std::mutex m_tree_mutex_;
    OutputFileService* m_output_svc = nullptr;
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
