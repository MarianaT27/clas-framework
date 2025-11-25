#pragma once

#include "Framework/IService.hpp"
#include "Framework/IAlgorithm.hpp"
#include <fmt/format.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <memory>

namespace Framework {

/**
 * @brief Service for visualizing algorithm workflows and dependencies
 *
 * Features:
 * - Generate DOT/GraphViz diagrams
 * - Show algorithm dependencies
 * - Visualize conditional execution
 * - Export execution order
 * - Analyze data flow
 * - Detect circular dependencies
 *
 * Example:
 * @code
 * auto viz = service<WorkflowVisualizationService>("WorkflowViz");
 *
 * // Register algorithms and dependencies
 * viz->register_algorithm("ElectronFilter");
 * viz->register_algorithm("KinematicsCalculator", {"ElectronFilter"});
 * viz->register_conditional("ElectronFilter", "KinematicsCalculator");
 *
 * // Generate visualization
 * viz->generate_dot("workflow.dot");
 * viz->print_execution_order();
 * @endcode
 */
class WorkflowVisualizationService : public Service {
public:
    /**
     * @brief Node type in the workflow
     */
    enum class NodeType {
        ALGORITHM,        // Regular algorithm
        FILTER,          // Filter (conditional)
        SERVICE,         // Service
        DATA_OBJECT      // Data object in Event store
    };

    /**
     * @brief Edge type (relationship between nodes)
     */
    enum class EdgeType {
        EXECUTION_ORDER,  // Sequential execution
        DEPENDENCY,       // Data dependency
        CONDITIONAL,      // Conditional execution
        SERVICE_USE       // Algorithm uses service
    };

    struct Node {
        std::string name;
        NodeType type;
        std::string description;
        std::set<std::string> inputs;   // Data inputs
        std::set<std::string> outputs;  // Data outputs
    };

    struct Edge {
        std::string from;
        std::string to;
        EdgeType type;
        std::string label;
    };

    explicit WorkflowVisualizationService(std::string name = "WorkflowViz")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        if (!m_output_file.empty()) {
            generate_dot(m_output_file);
        }
        return StatusCode::SUCCESS();
    }

    /**
     * @brief Set output DOT file
     */
    auto set_output_file(const std::string& filename) -> void {
        m_output_file = filename;
    }

    /**
     * @brief Register an algorithm
     */
    auto register_algorithm(const std::string& name,
                           const std::vector<std::string>& dependencies = {},
                           NodeType type = NodeType::ALGORITHM) -> void {
        Node node;
        node.name = name;
        node.type = type;
        m_nodes[name] = node;

        // Add dependency edges
        for (const auto& dep : dependencies) {
            Edge edge;
            edge.from = dep;
            edge.to = name;
            edge.type = EdgeType::DEPENDENCY;
            m_edges.push_back(edge);
        }
    }

    /**
     * @brief Register a filter (conditional algorithm)
     */
    auto register_filter(const std::string& name,
                        const std::vector<std::string>& dependencies = {}) -> void {
        register_algorithm(name, dependencies, NodeType::FILTER);
    }

    /**
     * @brief Register execution order
     */
    auto register_execution_order(const std::string& first,
                                  const std::string& second) -> void {
        Edge edge;
        edge.from = first;
        edge.to = second;
        edge.type = EdgeType::EXECUTION_ORDER;
        m_edges.push_back(edge);
    }

    /**
     * @brief Register conditional execution
     */
    auto register_conditional(const std::string& condition,
                             const std::string& algorithm) -> void {
        Edge edge;
        edge.from = condition;
        edge.to = algorithm;
        edge.type = EdgeType::CONDITIONAL;
        edge.label = "if passes";
        m_edges.push_back(edge);
    }

    /**
     * @brief Register service usage
     */
    auto register_service_use(const std::string& algorithm,
                             const std::string& service) -> void {
        // Register service as node if not already
        if (m_nodes.find(service) == m_nodes.end()) {
            Node node;
            node.name = service;
            node.type = NodeType::SERVICE;
            m_nodes[service] = node;
        }

        Edge edge;
        edge.from = algorithm;
        edge.to = service;
        edge.type = EdgeType::SERVICE_USE;
        edge.label = "uses";
        m_edges.push_back(edge);
    }

    /**
     * @brief Register data flow
     */
    auto register_data_flow(const std::string& producer,
                           const std::string& data_object,
                           const std::string& consumer) -> void {
        // Add data object as output of producer
        m_nodes[producer].outputs.insert(data_object);

        // Add data object as input of consumer
        m_nodes[consumer].inputs.insert(data_object);

        // Register data object node
        if (m_nodes.find(data_object) == m_nodes.end()) {
            Node node;
            node.name = data_object;
            node.type = NodeType::DATA_OBJECT;
            m_nodes[data_object] = node;
        }

        // Add edges
        Edge edge1;
        edge1.from = producer;
        edge1.to = data_object;
        edge1.type = EdgeType::DEPENDENCY;
        edge1.label = "produces";
        m_edges.push_back(edge1);

        Edge edge2;
        edge2.from = data_object;
        edge2.to = consumer;
        edge2.type = EdgeType::DEPENDENCY;
        edge2.label = "consumes";
        m_edges.push_back(edge2);
    }

    /**
     * @brief Set node description
     */
    auto set_description(const std::string& name, const std::string& desc) -> void {
        if (m_nodes.find(name) != m_nodes.end()) {
            m_nodes[name].description = desc;
        }
    }

    /**
     * @brief Generate DOT file for GraphViz
     */
    auto generate_dot(const std::string& filename) -> bool {
        std::ofstream file(filename);
        if (!file.is_open()) {
            fmt::print(stderr, "ERROR: Failed to open file for writing: {}\n", filename);
            return false;
        }

        file << "digraph Workflow {\n";
        file << "  // Graph settings\n";
        file << "  rankdir=TB;\n";
        file << "  node [shape=box, style=rounded];\n";
        file << "  edge [fontsize=10];\n";
        file << "  graph [fontname=\"Helvetica\", fontsize=12];\n";
        file << "  node [fontname=\"Helvetica\", fontsize=11];\n";
        file << "  edge [fontname=\"Helvetica\"];\n\n";

        // Write nodes
        file << "  // Nodes\n";
        for (const auto& [name, node] : m_nodes) {
            std::string color, shape;
            get_node_style(node.type, color, shape);

            file << fmt::format("  \"{}\" [", name);
            file << fmt::format("label=\"{}\"", name);
            file << fmt::format(", fillcolor=\"{}\", style=\"filled,rounded\"", color);
            file << fmt::format(", shape={}", shape);

            if (!node.description.empty()) {
                file << fmt::format(", tooltip=\"{}\"", node.description);
            }

            file << "];\n";
        }

        file << "\n  // Edges\n";
        for (const auto& edge : m_edges) {
            std::string style, color;
            get_edge_style(edge.type, style, color);

            file << fmt::format("  \"{}\" -> \"{}\" [", edge.from, edge.to);
            file << fmt::format("style={}, color=\"{}\"", style, color);

            if (!edge.label.empty()) {
                file << fmt::format(", label=\"{}\"", edge.label);
            }

            file << "];\n";
        }

        // Add subgraphs for clustering
        file << "\n  // Clustering\n";
        file << "  subgraph cluster_algorithms {\n";
        file << "    label=\"Algorithms\";\n";
        file << "    style=dashed;\n";
        file << "    color=gray;\n";
        for (const auto& [name, node] : m_nodes) {
            if (node.type == NodeType::ALGORITHM || node.type == NodeType::FILTER) {
                file << fmt::format("    \"{}\";\n", name);
            }
        }
        file << "  }\n";

        file << "\n  subgraph cluster_services {\n";
        file << "    label=\"Services\";\n";
        file << "    style=dashed;\n";
        file << "    color=gray;\n";
        for (const auto& [name, node] : m_nodes) {
            if (node.type == NodeType::SERVICE) {
                file << fmt::format("    \"{}\";\n", name);
            }
        }
        file << "  }\n";

        file << "}\n";
        file.close();

        fmt::print("✓ Generated workflow visualization: {}\n", filename);
        fmt::print("  Render with: dot -Tpng {} -o workflow.png\n", filename);
        fmt::print("  Or view with: xdot {}\n", filename);

        return true;
    }

    /**
     * @brief Print execution order to console
     */
    auto print_execution_order() const -> void {
        fmt::print("\n");
        fmt::print("╔════════════════════════════════════════════════════════════════╗\n");
        fmt::print("║                    Workflow Execution Order                    ║\n");
        fmt::print("╚════════════════════════════════════════════════════════════════╝\n");
        fmt::print("\n");

        // Perform topological sort
        auto sorted = topological_sort();

        int step = 1;
        for (const auto& name : sorted) {
            auto it = m_nodes.find(name);
            if (it == m_nodes.end()) continue;

            const auto& node = it->second;
            std::string type_str = get_type_string(node.type);

            fmt::print("{}. {} [{}]\n", step++, name, type_str);

            if (!node.inputs.empty()) {
                fmt::print("   Inputs: {}\n", fmt::join(node.inputs, ", "));
            }
            if (!node.outputs.empty()) {
                fmt::print("   Outputs: {}\n", fmt::join(node.outputs, ", "));
            }
        }

        fmt::print("\n");
    }

    /**
     * @brief Print statistics
     */
    auto print_statistics() const -> void {
        fmt::print("\nWorkflow Statistics:\n");
        fmt::print("  Total nodes: {}\n", m_nodes.size());
        fmt::print("  Total edges: {}\n", m_edges.size());

        size_t algorithms = 0, filters = 0, services = 0, data_objects = 0;
        for (const auto& [name, node] : m_nodes) {
            switch (node.type) {
                case NodeType::ALGORITHM: algorithms++; break;
                case NodeType::FILTER: filters++; break;
                case NodeType::SERVICE: services++; break;
                case NodeType::DATA_OBJECT: data_objects++; break;
            }
        }

        fmt::print("    Algorithms: {}\n", algorithms);
        fmt::print("    Filters: {}\n", filters);
        fmt::print("    Services: {}\n", services);
        fmt::print("    Data objects: {}\n", data_objects);

        // Detect cycles
        if (has_cycle()) {
            fmt::print("  ⚠️  WARNING: Circular dependencies detected!\n");
        } else {
            fmt::print("  ✓ No circular dependencies\n");
        }

        fmt::print("\n");
    }

    /**
     * @brief Check for circular dependencies
     */
    auto has_cycle() const -> bool {
        std::set<std::string> visited;
        std::set<std::string> rec_stack;

        for (const auto& [name, node] : m_nodes) {
            if (has_cycle_util(name, visited, rec_stack)) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Get execution complexity estimate
     */
    auto get_complexity_estimate() const -> std::string {
        size_t total_nodes = m_nodes.size();
        size_t total_edges = m_edges.size();

        if (total_nodes < 5) return "Simple";
        if (total_nodes < 15) return "Moderate";
        if (total_nodes < 30) return "Complex";
        return "Very Complex";
    }

private:
    std::string m_output_file;
    std::map<std::string, Node> m_nodes;
    std::vector<Edge> m_edges;

    auto get_node_style(NodeType type, std::string& color, std::string& shape) const -> void {
        switch (type) {
            case NodeType::ALGORITHM:
                color = "lightblue";
                shape = "box";
                break;
            case NodeType::FILTER:
                color = "lightyellow";
                shape = "diamond";
                break;
            case NodeType::SERVICE:
                color = "lightgreen";
                shape = "ellipse";
                break;
            case NodeType::DATA_OBJECT:
                color = "pink";
                shape = "folder";
                break;
        }
    }

    auto get_edge_style(EdgeType type, std::string& style, std::string& color) const -> void {
        switch (type) {
            case EdgeType::EXECUTION_ORDER:
                style = "solid";
                color = "black";
                break;
            case EdgeType::DEPENDENCY:
                style = "solid";
                color = "blue";
                break;
            case EdgeType::CONDITIONAL:
                style = "dashed";
                color = "orange";
                break;
            case EdgeType::SERVICE_USE:
                style = "dotted";
                color = "green";
                break;
        }
    }

    auto get_type_string(NodeType type) const -> std::string {
        switch (type) {
            case NodeType::ALGORITHM: return "Algorithm";
            case NodeType::FILTER: return "Filter";
            case NodeType::SERVICE: return "Service";
            case NodeType::DATA_OBJECT: return "Data";
        }
        return "Unknown";
    }

    auto topological_sort() const -> std::vector<std::string> {
        std::map<std::string, int> in_degree;
        std::map<std::string, std::vector<std::string>> adj_list;

        // Initialize
        for (const auto& [name, node] : m_nodes) {
            in_degree[name] = 0;
        }

        // Build adjacency list and in-degree
        for (const auto& edge : m_edges) {
            if (edge.type == EdgeType::EXECUTION_ORDER ||
                edge.type == EdgeType::DEPENDENCY) {
                adj_list[edge.from].push_back(edge.to);
                in_degree[edge.to]++;
            }
        }

        // Kahn's algorithm
        std::vector<std::string> result;
        std::vector<std::string> queue;

        for (const auto& [name, degree] : in_degree) {
            if (degree == 0) {
                queue.push_back(name);
            }
        }

        while (!queue.empty()) {
            std::string current = queue.back();
            queue.pop_back();
            result.push_back(current);

            for (const auto& neighbor : adj_list[current]) {
                in_degree[neighbor]--;
                if (in_degree[neighbor] == 0) {
                    queue.push_back(neighbor);
                }
            }
        }

        return result;
    }

    auto has_cycle_util(const std::string& node,
                       std::set<std::string>& visited,
                       std::set<std::string>& rec_stack) const -> bool {
        if (rec_stack.find(node) != rec_stack.end()) {
            return true;  // Back edge found
        }

        if (visited.find(node) != visited.end()) {
            return false;  // Already processed
        }

        visited.insert(node);
        rec_stack.insert(node);

        // Check all neighbors
        for (const auto& edge : m_edges) {
            if (edge.from == node &&
                (edge.type == EdgeType::EXECUTION_ORDER ||
                 edge.type == EdgeType::DEPENDENCY)) {
                if (has_cycle_util(edge.to, visited, rec_stack)) {
                    return true;
                }
            }
        }

        rec_stack.erase(node);
        return false;
    }
};

/**
 * @brief Helper for automatic workflow registration
 */
class WorkflowBuilder {
public:
    explicit WorkflowBuilder(WorkflowVisualizationService* service)
        : m_service(service) {}

    auto add_algorithm(const std::string& name) -> WorkflowBuilder& {
        m_service->register_algorithm(name);
        m_last_node = name;
        return *this;
    }

    auto add_filter(const std::string& name) -> WorkflowBuilder& {
        m_service->register_filter(name);
        m_last_node = name;
        return *this;
    }

    auto then(const std::string& next) -> WorkflowBuilder& {
        if (!m_last_node.empty()) {
            m_service->register_execution_order(m_last_node, next);
        }
        m_service->register_algorithm(next);
        m_last_node = next;
        return *this;
    }

    auto if_passes(const std::string& algorithm) -> WorkflowBuilder& {
        if (!m_last_node.empty()) {
            m_service->register_conditional(m_last_node, algorithm);
        }
        m_last_node = algorithm;
        return *this;
    }

    auto uses_service(const std::string& service) -> WorkflowBuilder& {
        if (!m_last_node.empty()) {
            m_service->register_service_use(m_last_node, service);
        }
        return *this;
    }

    auto produces(const std::string& data_object) -> WorkflowBuilder& {
        m_producing_object = data_object;
        return *this;
    }

    auto consumed_by(const std::string& consumer) -> WorkflowBuilder& {
        if (!m_producing_object.empty() && !m_last_node.empty()) {
            m_service->register_data_flow(m_last_node, m_producing_object, consumer);
            m_producing_object.clear();
        }
        return *this;
    }

private:
    WorkflowVisualizationService* m_service;
    std::string m_last_node;
    std::string m_producing_object;
};

}  // namespace Framework
