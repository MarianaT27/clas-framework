#include <gtest/gtest.h>

#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/Inject.hpp>
#include <clas12/framework/core/ServiceManager.hpp>

using namespace clas12::framework;

// Test service for unit tests
class TestInjectService : public Service {
public:
    explicit TestInjectService(std::string name = "TestInjectService")
        : Service(std::move(name)) {}

    auto do_something() const -> int { return 42; }
};

// Another test service
class AnotherInjectService : public Service {
public:
    explicit AnotherInjectService(std::string name = "AnotherInjectService")
        : Service(std::move(name)) {}

    auto get_value() const -> std::string { return "hello"; }
};

// Test fixture
class InjectTest : public ::testing::Test {
protected:
    void SetUp() override {
        svc_mgr = std::make_unique<ServiceManager>();
    }

    std::unique_ptr<ServiceManager> svc_mgr;
};

// ============================================================================
// Inject<T> Tests (Requirement 12.1)
// ============================================================================

TEST_F(InjectTest, RequiredInjectionResolves) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    Inject<TestInjectService> injection("TestService");
    auto sc = injection.resolve(*svc_mgr);

    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(injection.is_resolved());
    EXPECT_NE(injection.get(), nullptr);
}

TEST_F(InjectTest, RequiredInjectionFailsWhenMissing) {
    Inject<TestInjectService> injection("NonExistentService");
    auto sc = injection.resolve(*svc_mgr);

    EXPECT_TRUE(sc.is_failure());
    EXPECT_FALSE(injection.is_resolved());
    EXPECT_EQ(injection.get(), nullptr);
    EXPECT_NE(sc.message().find("NonExistentService"), std::string::npos);
}

TEST_F(InjectTest, OptionalInjectionSucceedsWhenMissing) {
    Inject<TestInjectService> injection("NonExistentService", false);
    auto sc = injection.resolve(*svc_mgr);

    EXPECT_TRUE(sc.is_success());
    EXPECT_FALSE(injection.is_resolved());
    EXPECT_EQ(injection.get(), nullptr);
}

TEST_F(InjectTest, OptionalInjectionResolvesWhenPresent) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    Inject<TestInjectService> injection("TestService", false);
    auto sc = injection.resolve(*svc_mgr);

    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(injection.is_resolved());
    EXPECT_NE(injection.get(), nullptr);
}

TEST_F(InjectTest, ArrowOperator) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    Inject<TestInjectService> injection("TestService");
    (void)injection.resolve(*svc_mgr);

    EXPECT_EQ(injection->do_something(), 42);
    EXPECT_EQ(injection->name(), "TestService");
}

TEST_F(InjectTest, DereferenceOperator) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    Inject<TestInjectService> injection("TestService");
    (void)injection.resolve(*svc_mgr);

    TestInjectService& svc_ref = *injection;
    EXPECT_EQ(svc_ref.name(), "TestService");
    EXPECT_EQ(svc_ref.do_something(), 42);
}

TEST_F(InjectTest, BoolConversion) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    Inject<TestInjectService> resolved("TestService");
    (void)resolved.resolve(*svc_mgr);

    Inject<TestInjectService> unresolved("NonExistent", false);
    (void)unresolved.resolve(*svc_mgr);

    EXPECT_TRUE(static_cast<bool>(resolved));
    EXPECT_FALSE(static_cast<bool>(unresolved));

    // Test in if statement
    if (resolved) {
        SUCCEED();
    } else {
        FAIL() << "Resolved injection should convert to true";
    }

    if (unresolved) {
        FAIL() << "Unresolved injection should convert to false";
    } else {
        SUCCEED();
    }
}

TEST_F(InjectTest, IsRequiredFlag) {
    Inject<TestInjectService> required("Service1");
    Inject<TestInjectService> optional("Service2", false);

    EXPECT_TRUE(required.is_required());
    EXPECT_FALSE(optional.is_required());
}

TEST_F(InjectTest, NameAccessor) {
    Inject<TestInjectService> injection("MyServiceName");
    EXPECT_EQ(injection.name(), "MyServiceName");
}


// ============================================================================
// InjectableAlgorithm Tests (Requirements 12.2, 12.3, 12.4)
// ============================================================================

// Mock Event class for testing
class MockEvent {
public:
    int event_number = 0;
};

// Simple injectable algorithm for testing
class SimpleInjectableAlg : public InjectableAlgorithm<SimpleInjectableAlg> {
public:
    SimpleInjectableAlg() : InjectableAlgorithm("SimpleInjectableAlg") {
        register_injection(m_test_svc);
    }

    auto process(Event& /*event*/) -> StatusCode override {
        if (m_test_svc) {
            m_last_value = m_test_svc->do_something();
        }
        return StatusCode::SUCCESS();
    }

    auto get_last_value() const -> int { return m_last_value; }
    auto get_service() -> TestInjectService* { return m_test_svc.get(); }

private:
    Inject<TestInjectService> m_test_svc{"TestService"};
    int m_last_value = 0;
};

// Algorithm with optional dependency
class OptionalDependencyAlg : public InjectableAlgorithm<OptionalDependencyAlg> {
public:
    OptionalDependencyAlg() : InjectableAlgorithm("OptionalDependencyAlg") {
        register_injection(m_required_svc);
        register_injection(m_optional_svc);
    }

    auto process(Event& /*event*/) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto has_required() const -> bool { return static_cast<bool>(m_required_svc); }
    auto has_optional() const -> bool { return static_cast<bool>(m_optional_svc); }

private:
    Inject<TestInjectService> m_required_svc{"TestService"};
    Inject<AnotherInjectService> m_optional_svc{"OptionalService", false};
};

// Algorithm with multiple required dependencies
class MultiDependencyAlg : public InjectableAlgorithm<MultiDependencyAlg> {
public:
    MultiDependencyAlg() : InjectableAlgorithm("MultiDependencyAlg") {
        register_injection(m_svc1);
        register_injection(m_svc2);
    }

    auto process(Event& /*event*/) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    auto on_initialize() -> StatusCode override {
        m_initialized = true;
        return StatusCode::SUCCESS();
    }

    auto on_finalize() -> StatusCode override {
        m_finalized = true;
        return StatusCode::SUCCESS();
    }

    auto is_initialized() const -> bool { return m_initialized; }
    auto is_finalized() const -> bool { return m_finalized; }

private:
    Inject<TestInjectService> m_svc1{"TestService"};
    Inject<AnotherInjectService> m_svc2{"AnotherService"};
    bool m_initialized = false;
    bool m_finalized = false;
};

// Test fixture for InjectableAlgorithm
class InjectableAlgorithmTest : public ::testing::Test {
protected:
    void SetUp() override {
        svc_mgr = std::make_unique<ServiceManager>();
    }

    std::unique_ptr<ServiceManager> svc_mgr;
};

TEST_F(InjectableAlgorithmTest, InitializeWithRequiredService) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));

    SimpleInjectableAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_success());
    EXPECT_NE(alg.get_service(), nullptr);
}

TEST_F(InjectableAlgorithmTest, InitializeFailsWithMissingRequiredService) {
    SimpleInjectableAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("TestService"), std::string::npos);
}

TEST_F(InjectableAlgorithmTest, InitializeFailsWithNoServiceManager) {
    SimpleInjectableAlg alg;
    // Don't set service manager

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("ServiceManager"), std::string::npos);
}

TEST_F(InjectableAlgorithmTest, OptionalDependencySucceedsWhenMissing) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));
    // Don't add OptionalService

    OptionalDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.has_required());
    EXPECT_FALSE(alg.has_optional());
}

TEST_F(InjectableAlgorithmTest, OptionalDependencyResolvesWhenPresent) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    auto optional_svc = std::make_unique<AnotherInjectService>("OptionalService");
    (void)svc_mgr->add_service(std::move(test_svc));
    (void)svc_mgr->add_service(std::move(optional_svc));

    OptionalDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(alg.has_required());
    EXPECT_TRUE(alg.has_optional());
}

TEST_F(InjectableAlgorithmTest, MultipleRequiredDependencies) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    auto another_svc = std::make_unique<AnotherInjectService>("AnotherService");
    (void)svc_mgr->add_service(std::move(test_svc));
    (void)svc_mgr->add_service(std::move(another_svc));

    MultiDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_success());
}

TEST_F(InjectableAlgorithmTest, MultipleRequiredDependenciesFailsIfOneMissing) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    (void)svc_mgr->add_service(std::move(test_svc));
    // Don't add AnotherService

    MultiDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_failure());
    EXPECT_NE(sc.message().find("AnotherService"), std::string::npos);
}

TEST_F(InjectableAlgorithmTest, OnInitializeCalled) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    auto another_svc = std::make_unique<AnotherInjectService>("AnotherService");
    (void)svc_mgr->add_service(std::move(test_svc));
    (void)svc_mgr->add_service(std::move(another_svc));

    MultiDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());

    EXPECT_FALSE(alg.is_initialized());
    (void)alg.initialize();
    EXPECT_TRUE(alg.is_initialized());
}

TEST_F(InjectableAlgorithmTest, OnFinalizeCalled) {
    auto test_svc = std::make_unique<TestInjectService>("TestService");
    auto another_svc = std::make_unique<AnotherInjectService>("AnotherService");
    (void)svc_mgr->add_service(std::move(test_svc));
    (void)svc_mgr->add_service(std::move(another_svc));

    MultiDependencyAlg alg;
    alg.set_service_manager(svc_mgr.get());
    (void)alg.initialize();

    EXPECT_FALSE(alg.is_finalized());
    (void)alg.finalize();
    EXPECT_TRUE(alg.is_finalized());
}

// Algorithm that exposes injection count for testing
class InjectionCountAlg : public InjectableAlgorithm<InjectionCountAlg> {
public:
    InjectionCountAlg() : InjectableAlgorithm("InjectionCountAlg") {
        register_injection(m_svc1);
        register_injection(m_svc2);
    }

    auto process(Event& /*event*/) -> StatusCode override {
        return StatusCode::SUCCESS();
    }

    // Expose injection_count for testing
    auto get_injection_count() const -> std::size_t {
        return injection_count();
    }

private:
    Inject<TestInjectService> m_svc1{"TestService"};
    Inject<AnotherInjectService> m_svc2{"AnotherService"};
};

TEST_F(InjectableAlgorithmTest, InjectionCount) {
    InjectionCountAlg alg;
    EXPECT_EQ(alg.get_injection_count(), 2);
}

TEST_F(InjectableAlgorithmTest, ClearErrorMessageForMissingService) {
    SimpleInjectableAlg alg;
    alg.set_service_manager(svc_mgr.get());

    auto sc = alg.initialize();
    EXPECT_TRUE(sc.is_failure());
    
    // Error message should contain algorithm name and service name
    EXPECT_NE(sc.message().find("SimpleInjectableAlg"), std::string::npos);
    EXPECT_NE(sc.message().find("TestService"), std::string::npos);
}
