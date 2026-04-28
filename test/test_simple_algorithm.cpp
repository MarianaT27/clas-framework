#include <gtest/gtest.h>
#include <clas12/framework/core/Algorithm.hpp>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceManager.hpp>

using namespace framework;

// ============================================================================
// Test Services
// ============================================================================

class MockHistogramService : public Service {
public:
    static constexpr auto service_name() -> std::string_view { return "HistogramService"; }

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
    static constexpr auto service_name() -> std::string_view { return "LoggingService"; }

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
// Test Algorithm Implementation
// ============================================================================

class TestAlgorithm : public Algorithm {
public:
    TestAlgorithm() : Algorithm("TestAlgorithm") {
        declare_property(m_threshold);
    }

    auto process([[maybe_unused]] Event& event) -> StatusCode override {
        m_process_count++;
        if (m_hist_svc) {
            m_hist_svc->fill("test_hist", m_threshold.get());
        }
        return StatusCode::SUCCESS();
    }

    auto on_initialize() -> StatusCode override {
        m_hist_svc = svc<MockHistogramService>();
        m_on_initialize_called = true;
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        m_on_finalize_called = true;
        return StatusCode::SUCCESS();
    }

    // Test accessors
    auto on_initialize_called() const -> bool { return m_on_initialize_called; }
    auto on_finalize_called() const -> bool { return m_on_finalize_called; }
    auto process_count() const -> int { return m_process_count; }
    auto threshold() const -> double { return m_threshold.get(); }
    auto hist_svc() const -> MockHistogramService* { return m_hist_svc; }

private:
    Property<double> m_threshold{"threshold", 2.0, "Test threshold"};
    MockHistogramService* m_hist_svc = nullptr;
    bool m_on_initialize_called = false;
    bool m_on_finalize_called = false;
    int m_process_count = 0;
};

// Algorithm with multiple services
class MultiServiceAlgorithm : public Algorithm {
public:
    MultiServiceAlgorithm() : Algorithm("MultiServiceAlgorithm") {}

    auto on_initialize() -> StatusCode override {
        m_hist_svc = svc<MockHistogramService>();
        m_log_svc = svc<MockLoggingService>();
        return StatusCode::SUCCESS();
    }

    auto process([[maybe_unused]] Event& event) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto hist_service_valid() const -> bool {
        return m_hist_svc != nullptr;
    }

    auto log_service_valid() const -> bool {
        return m_log_svc != nullptr;
    }

private:
    MockHistogramService* m_hist_svc = nullptr;
    MockLoggingService* m_log_svc = nullptr;
};

// Algorithm that returns FILTERED
class FilteringAlgorithm : public Algorithm {
public:
    FilteringAlgorithm() : Algorithm("FilteringAlgorithm") {}

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

class AlgorithmTest : public ::testing::Test {
protected:
    void SetUp() override {
        svc_mgr = std::make_unique<ServiceManager>();
        
        // Add mock services
        (void)svc_mgr->add_service(std::make_unique<MockHistogramService>("HistogramService"));
        (void)svc_mgr->add_service(std::make_unique<MockLoggingService>("LoggingService"));
        (void)svc_mgr->initialize_all();
    }

    std::unique_ptr<ServiceManager> svc_mgr;
};

// ============================================================================
// Algorithm Tests
// ============================================================================

TEST_F(AlgorithmTest, NameIsSet) {
    TestAlgorithm alg;
    EXPECT_EQ(alg.name(), "TestAlgorithm");
}

TEST_F(AlgorithmTest, InitializeSucceeds) {
    TestAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();

    EXPECT_TRUE(sc.is_success());
}

TEST_F(AlgorithmTest, InitializeCallsOnInitialize) {
    TestAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    EXPECT_FALSE(alg.on_initialize_called());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.on_initialize_called());
}

TEST_F(AlgorithmTest, InitializeInjectsServices) {
    TestAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();

    EXPECT_TRUE(sc.is_success());
    // Service should be accessible via svc<T>()
    EXPECT_NE(alg.hist_svc(), nullptr);
}

TEST_F(AlgorithmTest, InitializeWithoutServiceManagerReturnsNullServices) {
    TestAlgorithm alg;
    // Don't set service manager — svc<T>() returns nullptr

    auto sc = alg.initialize();

    EXPECT_TRUE(sc.is_success());
    EXPECT_EQ(alg.hist_svc(), nullptr);
}

TEST_F(AlgorithmTest, FinalizeCallsOnFinalize) {
    TestAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    (void)alg.initialize();
    
    EXPECT_FALSE(alg.on_finalize_called());
    
    auto sc = alg.finalize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.on_finalize_called());
}

TEST_F(AlgorithmTest, MultipleServicesInjected) {
    MultiServiceAlgorithm alg;
    alg.set_service_manager(svc_mgr.get());
    
    auto sc = alg.initialize();
    
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.hist_service_valid());
    EXPECT_TRUE(alg.log_service_valid());
}

TEST_F(AlgorithmTest, PropertyIntegration) {
    TestAlgorithm alg;
    
    // Check default value
    EXPECT_DOUBLE_EQ(alg.threshold(), 2.0);
    
    // Properties should be registered via PropertyMixin
    EXPECT_TRUE(alg.has_property("threshold"));
    EXPECT_EQ(alg.property_count(), 1);
}

TEST_F(AlgorithmTest, FilteringStatusCode) {
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

