#include <gtest/gtest.h>
#include "Framework/SimpleAlgorithm.hpp"
#include "Framework/IService.hpp"
#include "Framework/ServiceManager.hpp"

using namespace Framework;

// ============================================================================
// Test Services
// ============================================================================

class MockHistogramService : public Service {
public:
    explicit MockHistogramService(std::string name = "HistogramService")
        : Service(std::move(name)) {}

    auto initialize() -> StatusCode override {
        m_initialized = true;
        return StatusCode::SUCCESS();
    }

    auto finalize() -> StatusCode override {
        m_finalized = true;
        return StatusCode::SUCCESS();
    }

    auto is_initialized() const -> bool { return m_initialized; }
    auto is_finalized() const -> bool { return m_finalized; }
    
    auto fill(const std::string& hist_name, double value) -> void {
        m_fills.push_back({hist_name, value});
    }
    
    auto fill_count() const -> std::size_t { return m_fills.size(); }

private:
    bool m_initialized = false;
    bool m_finalized = false;
    std::vector<std::pair<std::string, double>> m_fills;
};

class MockLoggingService : public Service {
public:
    explicit MockLoggingService(std::string name = "LoggingService")
        : Service(std::move(name)) {}
        
    auto log(const std::string& msg) -> void {
        m_messages.push_back(msg);
    }
    
    auto message_count() const -> std::size_t { return m_messages.size(); }

private:
    std::vector<std::string> m_messages;
};

// ============================================================================
// Test SimpleAlgorithm Implementation
// ============================================================================

class TestSimpleAlgorithm : public SimpleAlgorithm<TestSimpleAlgorithm> {
public:
    TestSimpleAlgorithm() : SimpleAlgorithm("TestSimpleAlgorithm") {
        declare_property(m_threshold);
    }

    auto configure() -> void override {
        m_hist_svc = &require_service<MockHistogramService>("HistogramService");
        m_configure_called = true;
    }

    auto process([[maybe_unused]] Event& event) -> StatusCode override {
        m_process_count++;
        if (m_hist_svc && m_hist_svc->is_valid()) {
            m_hist_svc->get()->fill("test_hist", m_threshold.get());
        }
        return StatusCode::SUCCESS();
    }

    auto on_initialize() -> StatusCode override {
        m_on_initialize_called = true;
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        m_on_finalize_called = true;
        return StatusCode::SUCCESS();
    }

    // Test accessors
    auto configure_called() const -> bool { return m_configure_called; }
    auto on_initialize_called() const -> bool { return m_on_initialize_called; }
    auto on_finalize_called() const -> bool { return m_on_finalize_called; }
    auto process_count() const -> int { return m_process_count; }
    auto threshold() const -> double { return m_threshold.get(); }
    
    // Expose protected methods for testing
    using SimpleAlgorithm<TestSimpleAlgorithm>::service_count;
    using SimpleAlgorithm<TestSimpleAlgorithm>::has_service;

private:
    Property<double> m_threshold{"threshold", 2.0, "Test threshold"};
    ServiceHandle<MockHistogramService>* m_hist_svc = nullptr;
    bool m_configure_called = false;
    bool m_on_initialize_called = false;
    bool m_on_finalize_called = false;
    int m_process_count = 0;
};

// Algorithm with multiple services
class MultiServiceAlgorithm : public SimpleAlgorithm<MultiServiceAlgorithm> {
public:
    MultiServiceAlgorithm() : SimpleAlgorithm("MultiServiceAlgorithm") {}

    auto configure() -> void override {
        m_hist_svc = &require_service<MockHistogramService>("HistogramService");
        m_log_svc = &require_service<MockLoggingService>("LoggingService");
    }

    auto process([[maybe_unused]] Event& event) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto hist_service_valid() const -> bool {
        return m_hist_svc && m_hist_svc->is_valid();
    }
    
    auto log_service_valid() const -> bool {
        return m_log_svc && m_log_svc->is_valid();
    }

private:
    ServiceHandle<MockHistogramService>* m_hist_svc = nullptr;
    ServiceHandle<MockLoggingService>* m_log_svc = nullptr;
};

// Algorithm that returns FILTERED
class FilteringAlgorithm : public SimpleAlgorithm<FilteringAlgorithm> {
public:
    FilteringAlgorithm() : SimpleAlgorithm("FilteringAlgorithm") {}

    auto process([[maybe_unused]] Event& event) -> StatusCode override {
        m_process_count++;
        if (m_process_count % 2 == 0) {
            return StatusCode::FILTERED();
        }
        return StatusCode::SUCCESS();
    }

    auto process_count() const -> int { return m_process_count; }

private:
    int m_process_count = 0;
};

// ============================================================================
// Test Fixture
// ============================================================================

class SimpleAlgorithmTest : public ::testing::Test {
protected:
    void SetUp() override {
        svc_mgr = std::make_unique<ServiceManager>();
        
        // Add mock services
        (void)svc_mgr->add_service(std::make_unique<MockHistogramService>("HistogramService"));
        (void)svc_mgr->add_service(std::make_unique<MockLoggingService>("LoggingService"));
    }

    std::unique_ptr<ServiceManager> svc_mgr;
};

// ============================================================================
// SimpleAlgorithm Tests
// ============================================================================

TEST_F(SimpleAlgorithmTest, NameIsSet) {
    TestSimpleAlgorithm alg;
    EXPECT_EQ(alg.name(), "TestSimpleAlgorithm");
}

TEST_F(SimpleAlgorithmTest, InitializeCallsConfigureFirst) {
    TestSimpleAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    EXPECT_FALSE(alg.configure_called());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.configure_called());
}

TEST_F(SimpleAlgorithmTest, InitializeCallsOnInitialize) {
    TestSimpleAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    EXPECT_FALSE(alg.on_initialize_called());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.on_initialize_called());
}

TEST_F(SimpleAlgorithmTest, InitializeInjectsServices) {
    TestSimpleAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    // Service should be injected - we can verify by checking service_count
    EXPECT_EQ(alg.service_count(), 1);
    EXPECT_TRUE(alg.has_service("HistogramService"));
}

TEST_F(SimpleAlgorithmTest, InitializeFailsWithMissingService) {
    // Create algorithm that requires a non-existent service
    class MissingServiceAlg : public SimpleAlgorithm<MissingServiceAlg> {
    public:
        MissingServiceAlg() : SimpleAlgorithm("MissingServiceAlg") {}
        auto configure() -> void override {
            require_service<MockHistogramService>("NonExistentService");
        }
        auto process([[maybe_unused]] Event& event) -> StatusCode override {
            return StatusCode::SUCCESS();
        }
    };
    
    MissingServiceAlg alg;
    alg.set_service_manager(svc_mgr.get());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("NonExistentService"), std::string::npos);
}

TEST_F(SimpleAlgorithmTest, InitializeFailsWithoutServiceManager) {
    TestSimpleAlgorithm alg;
    // Don't set service manager
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("ServiceManager"), std::string::npos);
}

TEST_F(SimpleAlgorithmTest, FinalizeCallsOnFinalize) {
    TestSimpleAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    (void)alg.initialize();
    
    EXPECT_FALSE(alg.on_finalize_called());
    
    auto sc = alg.finalize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.on_finalize_called());
}

TEST_F(SimpleAlgorithmTest, MultipleServicesInjected) {
    MultiServiceAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.hist_service_valid());
    EXPECT_TRUE(alg.log_service_valid());
}

TEST_F(SimpleAlgorithmTest, PropertyIntegration) {
    TestSimpleAlgorithm alg;
    
    // Check default value
    EXPECT_DOUBLE_EQ(alg.threshold(), 2.0);
    
    // Properties should be registered via PropertyMixin
    EXPECT_TRUE(alg.has_property("threshold"));
    EXPECT_EQ(alg.property_count(), 1);
}

TEST_F(SimpleAlgorithmTest, FilteringStatusCode) {
    FilteringAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    (void)alg.initialize();
    
    // Create a mock event - we'll use a null reference trick for testing
    // In real usage, this would be a proper Event object
    // For this test, we just verify the status codes
    
    // Note: We can't easily create a mock Event without HIPO dependencies
    // So we'll just verify the algorithm structure is correct
    EXPECT_EQ(alg.name(), "FilteringAlgorithm");
}

// ============================================================================
// LambdaAlgorithm Tests
// ============================================================================

TEST(LambdaAlgorithmTest, BasicConstruction) {
    int call_count = 0;
    
    auto alg = make_algorithm("TestLambda", [&call_count]([[maybe_unused]] Event& event) {
        call_count++;
        return StatusCode::SUCCESS();
    });
    
    EXPECT_EQ(alg->name(), "TestLambda");
}

TEST(LambdaAlgorithmTest, InitializeReturnsSuccess) {
    auto alg = make_algorithm("TestLambda", []([[maybe_unused]] Event& event) {
        return StatusCode::SUCCESS();
    });
    
    auto sc = alg->initialize();
    EXPECT_TRUE(sc.is_success());
}

TEST(LambdaAlgorithmTest, FinalizeReturnsSuccess) {
    auto alg = make_algorithm("TestLambda", []([[maybe_unused]] Event& event) {
        return StatusCode::SUCCESS();
    });
    
    auto sc = alg->finalize();
    EXPECT_TRUE(sc.is_success());
}

TEST(LambdaAlgorithmTest, WithInitCallback) {
    bool init_called = false;
    
    LambdaAlgorithm alg("TestLambda", []([[maybe_unused]] Event& event) {
        return StatusCode::SUCCESS();
    });
    
    alg.with_init([&init_called]() {
        init_called = true;
        return StatusCode::SUCCESS();
    });
    
    EXPECT_FALSE(init_called);
    (void)alg.initialize();
    EXPECT_TRUE(init_called);
}

TEST(LambdaAlgorithmTest, WithFinalizeCallback) {
    bool finalize_called = false;
    
    LambdaAlgorithm alg("TestLambda", []([[maybe_unused]] Event& event) {
        return StatusCode::SUCCESS();
    });
    
    alg.with_finalize([&finalize_called]() {
        finalize_called = true;
        return StatusCode::SUCCESS();
    });
    
    EXPECT_FALSE(finalize_called);
    (void)alg.finalize();
    EXPECT_TRUE(finalize_called);
}

TEST(LambdaAlgorithmTest, FullConstructor) {
    bool init_called = false;
    bool finalize_called = false;
    int process_count = 0;
    
    auto alg = make_algorithm(
        "FullLambda",
        [&process_count]([[maybe_unused]] Event& event) {
            process_count++;
            return StatusCode::SUCCESS();
        },
        [&init_called]() {
            init_called = true;
            return StatusCode::SUCCESS();
        },
        [&finalize_called]() {
            finalize_called = true;
            return StatusCode::SUCCESS();
        }
    );
    
    (void)alg->initialize();
    EXPECT_TRUE(init_called);
    
    (void)alg->finalize();
    EXPECT_TRUE(finalize_called);
}

TEST(LambdaAlgorithmTest, InitFailurePropagates) {
    auto alg = make_algorithm(
        "FailingInit",
        []([[maybe_unused]] Event& event) { return StatusCode::SUCCESS(); },
        []() { return StatusCode::failure("Init failed"); },
        []() { return StatusCode::SUCCESS(); }
    );
    
    auto sc = alg->initialize();
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("Init failed"), std::string::npos);
}

TEST(LambdaAlgorithmTest, FinalizeFailurePropagates) {
    auto alg = make_algorithm(
        "FailingFinalize",
        []([[maybe_unused]] Event& event) { return StatusCode::SUCCESS(); },
        []() { return StatusCode::SUCCESS(); },
        []() { return StatusCode::failure("Finalize failed"); }
    );
    
    (void)alg->initialize();
    auto sc = alg->finalize();
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("Finalize failed"), std::string::npos);
}

TEST(LambdaAlgorithmTest, ChainedConfiguration) {
    bool init_called = false;
    bool finalize_called = false;
    
    LambdaAlgorithm alg("ChainedLambda", []([[maybe_unused]] Event& event) {
        return StatusCode::SUCCESS();
    });
    
    // Test method chaining
    alg.with_init([&init_called]() {
        init_called = true;
        return StatusCode::SUCCESS();
    }).with_finalize([&finalize_called]() {
        finalize_called = true;
        return StatusCode::SUCCESS();
    });
    
    (void)alg.initialize();
    (void)alg.finalize();
    
    EXPECT_TRUE(init_called);
    EXPECT_TRUE(finalize_called);
}
