#include <gtest/gtest.h>
#include <clas12/framework/core/IService.hpp>
#include <clas12/framework/core/ServiceHandle.hpp>
#include <clas12/framework/core/ServiceManager.hpp>

using namespace clas12::framework;

// Test service for unit tests
class TestService : public Service {
   public:
    explicit TestService(std::string name = "TestService")
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

    auto do_something() const -> int { return 42; }

   private:
    bool m_initialized = false;
    bool m_finalized = false;
};

// Another test service for type checking
class OtherService : public Service {
   public:
    explicit OtherService(std::string name = "OtherService")
        : Service(std::move(name)) {}
};

// Test fixture
class ServiceHandleTest : public ::testing::Test {
   protected:
    void SetUp() override {
        svc_mgr = std::make_unique<ServiceManager>();
    }

    std::unique_ptr<ServiceManager> svc_mgr;
};

TEST_F(ServiceHandleTest, InitializeWithValidService) {
    // Add a test service to the manager
    auto test_svc = std::make_unique<TestService>("MyTestService");
    svc_mgr->add_service(std::move(test_svc));

    // Create and initialize handle
    ServiceHandle<TestService> handle("MyTestService");
    auto sc = handle.initialize(*svc_mgr);

    EXPECT_TRUE(sc.is_success());
    EXPECT_TRUE(handle.is_valid());
    EXPECT_NE(handle.get(), nullptr);
}

TEST_F(ServiceHandleTest, InitializeWithMissingService) {
    // Don't add any service - handle should fail to initialize
    ServiceHandle<TestService> handle("NonExistentService");
    auto sc = handle.initialize(*svc_mgr);

    EXPECT_TRUE(sc.is_failure());
    EXPECT_FALSE(handle.is_valid());
    EXPECT_EQ(handle.get(), nullptr);
    // Check error message contains service name
    EXPECT_NE(sc.message().find("NonExistentService"), std::string::npos);
}

TEST_F(ServiceHandleTest, InitializeWithEmptyName) {
    ServiceHandle<TestService> handle("");
    auto sc = handle.initialize(*svc_mgr);

    EXPECT_TRUE(sc.is_failure());
    EXPECT_FALSE(handle.is_valid());
    EXPECT_NE(sc.message().find("empty"), std::string::npos);
}

TEST_F(ServiceHandleTest, ArrowOperator) {
    auto test_svc = std::make_unique<TestService>("TestSvc");
    svc_mgr->add_service(std::move(test_svc));

    ServiceHandle<TestService> handle("TestSvc");
    handle.initialize(*svc_mgr);

    // Use arrow operator to call method
    EXPECT_EQ(handle->do_something(), 42);
    EXPECT_EQ(handle->name(), "TestSvc");
}

TEST_F(ServiceHandleTest, DereferenceOperator) {
    auto test_svc = std::make_unique<TestService>("TestSvc");
    svc_mgr->add_service(std::move(test_svc));

    ServiceHandle<TestService> handle("TestSvc");
    handle.initialize(*svc_mgr);

    // Use dereference operator
    TestService& svc_ref = *handle;
    EXPECT_EQ(svc_ref.name(), "TestSvc");
    EXPECT_EQ(svc_ref.do_something(), 42);
}

TEST_F(ServiceHandleTest, GetMethod) {
    auto test_svc = std::make_unique<TestService>("TestSvc");
    svc_mgr->add_service(std::move(test_svc));

    ServiceHandle<TestService> handle("TestSvc");
    handle.initialize(*svc_mgr);

    TestService* ptr = handle.get();
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->name(), "TestSvc");
}

TEST_F(ServiceHandleTest, BoolConversion) {
    auto test_svc = std::make_unique<TestService>("TestSvc");
    svc_mgr->add_service(std::move(test_svc));

    ServiceHandle<TestService> valid_handle("TestSvc");
    valid_handle.initialize(*svc_mgr);

    ServiceHandle<TestService> invalid_handle("NonExistent");
    invalid_handle.initialize(*svc_mgr);

    // Test explicit bool conversion
    EXPECT_TRUE(static_cast<bool>(valid_handle));
    EXPECT_FALSE(static_cast<bool>(invalid_handle));

    // Test in if statement
    if (valid_handle) {
        SUCCEED();
    } else {
        FAIL() << "Valid handle should convert to true";
    }

    if (invalid_handle) {
        FAIL() << "Invalid handle should convert to false";
    } else {
        SUCCEED();
    }
}

TEST_F(ServiceHandleTest, IsValidMethod) {
    auto test_svc = std::make_unique<TestService>("TestSvc");
    svc_mgr->add_service(std::move(test_svc));

    ServiceHandle<TestService> handle("TestSvc");

    // Before initialization
    EXPECT_FALSE(handle.is_valid());

    // After successful initialization
    handle.initialize(*svc_mgr);
    EXPECT_TRUE(handle.is_valid());
}

TEST_F(ServiceHandleTest, NameAccessor) {
    ServiceHandle<TestService> handle("MyServiceName");
    EXPECT_EQ(handle.name(), "MyServiceName");
}

TEST_F(ServiceHandleTest, SetName) {
    ServiceHandle<TestService> handle;
    EXPECT_TRUE(handle.name().empty());

    handle.set_name("NewServiceName");
    EXPECT_EQ(handle.name(), "NewServiceName");
}

TEST_F(ServiceHandleTest, DefaultConstructor) {
    ServiceHandle<TestService> handle;

    EXPECT_TRUE(handle.name().empty());
    EXPECT_FALSE(handle.is_valid());
    EXPECT_EQ(handle.get(), nullptr);
}

TEST_F(ServiceHandleTest, WrongServiceType) {
    // Add OtherService but try to get it as TestService
    auto other_svc = std::make_unique<OtherService>("WrongType");
    svc_mgr->add_service(std::move(other_svc));

    ServiceHandle<TestService> handle("WrongType");
    auto sc = handle.initialize(*svc_mgr);

    // Should fail because dynamic_cast will return nullptr
    EXPECT_TRUE(sc.is_failure());
    EXPECT_FALSE(handle.is_valid());
}
