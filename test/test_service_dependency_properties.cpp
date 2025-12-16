#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <algorithm>
#include <set>
#include <string>
#include <vector>
#include "Framework/IService.hpp"
#include "Framework/ServiceManager.hpp"

using namespace Framework;

/**
 * Test service that tracks initialization order
 */
class TrackingService : public Service {
   public:
    explicit TrackingService(const std::string& name, std::vector<std::string>* init_order)
        : Service(name), m_init_order(init_order) {}

    auto initialize() -> StatusCode override {
        if (m_init_order) {
            m_init_order->push_back(m_name);
        }
        return StatusCode::SUCCESS();
    }

   private:
    std::vector<std::string>* m_init_order;
};

/**
 * **Feature: code-quality-improvements, Property 3: Service Dependency Ordering**
 *
 * *For any* set of services with declared dependencies forming a valid DAG (no cycles),
 * the ServiceManager SHALL initialize services such that each service is initialized
 * only after all its dependencies have been initialized.
 *
 * **Validates: Requirements 3.1, 3.3**
 */
RC_GTEST_PROP(ServiceDependencyProperties, DependenciesInitializedFirst, ()) {
    // Generate a small number of services (2-6) to keep test fast
    int num_services = *rc::gen::inRange(2, 7);

    // Generate service names
    std::vector<std::string> service_names;
    for (int i = 0; i < num_services; ++i) {
        service_names.push_back("Service" + std::to_string(i));
    }

    // Generate a valid DAG by only allowing dependencies on services with lower indices
    // This guarantees no cycles
    std::map<std::string, std::vector<std::string>> dependencies;
    for (int i = 1; i < num_services; ++i) {
        // Each service can depend on any subset of services with lower indices
        std::vector<std::string> deps;
        for (int j = 0; j < i; ++j) {
            // Randomly decide if this is a dependency
            if (*rc::gen::arbitrary<bool>()) {
                deps.push_back(service_names[j]);
            }
        }
        dependencies[service_names[i]] = deps;
    }
    dependencies[service_names[0]] = {};  // First service has no dependencies

    // Track initialization order
    std::vector<std::string> init_order;

    // Create ServiceManager and add services
    ServiceManager svc_mgr;
    for (const auto& name : service_names) {
        auto service = std::make_unique<TrackingService>(name, &init_order);
        auto deps = dependencies[name];
        auto sc = svc_mgr.add_service(std::move(service), deps);
        RC_ASSERT(sc.is_success());
    }

    // Initialize all services
    auto sc = svc_mgr.initialize_all();
    RC_ASSERT(sc.is_success());

    // Verify all services were initialized
    RC_ASSERT(init_order.size() == static_cast<size_t>(num_services));

    // Build a map of service name to initialization position
    std::map<std::string, size_t> init_position;
    for (size_t i = 0; i < init_order.size(); ++i) {
        init_position[init_order[i]] = i;
    }

    // Verify: for each service, all its dependencies were initialized before it
    for (const auto& [service, deps] : dependencies) {
        size_t service_pos = init_position[service];
        for (const auto& dep : deps) {
            size_t dep_pos = init_position[dep];
            RC_ASSERT_FALSE(dep_pos >= service_pos);  // Dependency must come before
        }
    }
}

/**
 * Property: Circular dependencies are detected and reported
 *
 * *For any* set of services with a circular dependency,
 * the ServiceManager SHALL detect the cycle and return an error.
 */
RC_GTEST_PROP(ServiceDependencyProperties, CircularDependencyDetection, ()) {
    // Create a simple cycle: A -> B -> C -> A
    ServiceManager svc_mgr;
    std::vector<std::string> init_order;

    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<TrackingService>("B", &init_order);
    auto svc_c = std::make_unique<TrackingService>("C", &init_order);

    svc_mgr.add_service(std::move(svc_a), {"C"});  // A depends on C
    svc_mgr.add_service(std::move(svc_b), {"A"});  // B depends on A
    svc_mgr.add_service(std::move(svc_c), {"B"});  // C depends on B (creates cycle)

    // topological_sort should detect the cycle
    auto result = svc_mgr.topological_sort();
    RC_ASSERT(result.is_error());
    RC_ASSERT(result.error_message().find("Circular dependency") != std::string::npos);

    // initialize_all should also fail
    auto sc = svc_mgr.initialize_all();
    RC_ASSERT(sc.is_failure());
}

// Unit tests for specific scenarios
TEST(ServiceDependencyTest, LinearDependencyChain) {
    std::vector<std::string> init_order;
    ServiceManager svc_mgr;

    // Create chain: A <- B <- C (C depends on B, B depends on A)
    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<TrackingService>("B", &init_order);
    auto svc_c = std::make_unique<TrackingService>("C", &init_order);

    svc_mgr.add_service(std::move(svc_a), {});
    svc_mgr.add_service(std::move(svc_b), {"A"});
    svc_mgr.add_service(std::move(svc_c), {"B"});

    auto sc = svc_mgr.initialize_all();
    EXPECT_TRUE(sc.is_success());

    // Verify order: A must come before B, B must come before C
    ASSERT_EQ(init_order.size(), 3);

    auto pos_a = std::find(init_order.begin(), init_order.end(), "A") - init_order.begin();
    auto pos_b = std::find(init_order.begin(), init_order.end(), "B") - init_order.begin();
    auto pos_c = std::find(init_order.begin(), init_order.end(), "C") - init_order.begin();

    EXPECT_LT(pos_a, pos_b);
    EXPECT_LT(pos_b, pos_c);
}

TEST(ServiceDependencyTest, DiamondDependency) {
    std::vector<std::string> init_order;
    ServiceManager svc_mgr;

    // Diamond: D depends on B and C, both B and C depend on A
    //     A
    //    / \
    //   B   C
    //    \ /
    //     D
    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<TrackingService>("B", &init_order);
    auto svc_c = std::make_unique<TrackingService>("C", &init_order);
    auto svc_d = std::make_unique<TrackingService>("D", &init_order);

    svc_mgr.add_service(std::move(svc_a), {});
    svc_mgr.add_service(std::move(svc_b), {"A"});
    svc_mgr.add_service(std::move(svc_c), {"A"});
    svc_mgr.add_service(std::move(svc_d), {"B", "C"});

    auto sc = svc_mgr.initialize_all();
    EXPECT_TRUE(sc.is_success());

    ASSERT_EQ(init_order.size(), 4);

    auto pos_a = std::find(init_order.begin(), init_order.end(), "A") - init_order.begin();
    auto pos_b = std::find(init_order.begin(), init_order.end(), "B") - init_order.begin();
    auto pos_c = std::find(init_order.begin(), init_order.end(), "C") - init_order.begin();
    auto pos_d = std::find(init_order.begin(), init_order.end(), "D") - init_order.begin();

    // A must come before B and C
    EXPECT_LT(pos_a, pos_b);
    EXPECT_LT(pos_a, pos_c);
    // B and C must come before D
    EXPECT_LT(pos_b, pos_d);
    EXPECT_LT(pos_c, pos_d);
}

TEST(ServiceDependencyTest, NonExistentDependency) {
    ServiceManager svc_mgr;
    std::vector<std::string> init_order;

    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    svc_mgr.add_service(std::move(svc_a), {"NonExistent"});

    auto result = svc_mgr.topological_sort();
    EXPECT_TRUE(result.is_error());
    EXPECT_TRUE(result.error_message().find("non-existent") != std::string::npos);
}

TEST(ServiceDependencyTest, NoDependencies) {
    std::vector<std::string> init_order;
    ServiceManager svc_mgr;

    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<TrackingService>("B", &init_order);

    svc_mgr.add_service(std::move(svc_a));
    svc_mgr.add_service(std::move(svc_b));

    auto sc = svc_mgr.initialize_all();
    EXPECT_TRUE(sc.is_success());
    EXPECT_EQ(init_order.size(), 2);
}

TEST(ServiceDependencyTest, GetDependencies) {
    ServiceManager svc_mgr;
    std::vector<std::string> init_order;

    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<TrackingService>("B", &init_order);

    svc_mgr.add_service(std::move(svc_a), {});
    svc_mgr.add_service(std::move(svc_b), {"A"});

    auto deps_a = svc_mgr.get_dependencies("A");
    auto deps_b = svc_mgr.get_dependencies("B");
    auto deps_unknown = svc_mgr.get_dependencies("Unknown");

    EXPECT_TRUE(deps_a.empty());
    ASSERT_EQ(deps_b.size(), 1);
    EXPECT_EQ(deps_b[0], "A");
    EXPECT_TRUE(deps_unknown.empty());
}

/**
 * Test service that fails initialization
 */
class FailingService : public Service {
   public:
    explicit FailingService(const std::string& name, const std::string& error_msg)
        : Service(name), m_error_msg(error_msg) {}

    auto initialize() -> StatusCode override {
        return StatusCode::failure(m_error_msg);
    }

   private:
    std::string m_error_msg;
};

TEST(ServiceDependencyTest, InitializationFailureReporting) {
    ServiceManager svc_mgr;
    std::vector<std::string> init_order;

    auto svc_a = std::make_unique<TrackingService>("A", &init_order);
    auto svc_b = std::make_unique<FailingService>("B", "Database connection failed");
    auto svc_c = std::make_unique<TrackingService>("C", &init_order);

    svc_mgr.add_service(std::move(svc_a), {});
    svc_mgr.add_service(std::move(svc_b), {"A"});
    svc_mgr.add_service(std::move(svc_c), {"B"});

    auto sc = svc_mgr.initialize_all();
    EXPECT_TRUE(sc.is_failure());
    // Verify error message contains service name and reason
    EXPECT_TRUE(sc.message().find("B") != std::string::npos);
    EXPECT_TRUE(sc.message().find("Database connection failed") != std::string::npos);

    // A should have been initialized before B failed
    EXPECT_EQ(init_order.size(), 1);
    EXPECT_EQ(init_order[0], "A");
}

TEST(ServiceDependencyTest, DuplicateServiceRejected) {
    ServiceManager svc_mgr;
    std::vector<std::string> init_order;

    auto svc_a1 = std::make_unique<TrackingService>("A", &init_order);
    auto svc_a2 = std::make_unique<TrackingService>("A", &init_order);

    auto sc1 = svc_mgr.add_service(std::move(svc_a1));
    EXPECT_TRUE(sc1.is_success());

    auto sc2 = svc_mgr.add_service(std::move(svc_a2));
    EXPECT_TRUE(sc2.is_failure());
    EXPECT_TRUE(sc2.message().find("already exists") != std::string::npos);
}
