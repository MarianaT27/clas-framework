#include <gtest/gtest.h>
#include <clas12/framework/core/ServiceManager.hpp>
#include <clas12/framework/services/ConditionsService.hpp>

using namespace framework;

TEST(ServiceManagerTest, AddAndRetrieveService) {
    ServiceManager svc_mgr;

    auto cond_svc = std::make_unique<ConditionsService>("TestConditions");
    svc_mgr.add_service(std::move(cond_svc));

    EXPECT_EQ(svc_mgr.size(), 1);

    auto* retrieved = svc_mgr.get_service<ConditionsService>("TestConditions");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->name(), "TestConditions");
}

TEST(ServiceManagerTest, NonExistentService) {
    ServiceManager svc_mgr;
    auto* service = svc_mgr.get_service<ConditionsService>("DoesNotExist");
    EXPECT_EQ(service, nullptr);
}

TEST(ServiceManagerTest, InitializeAll) {
    ServiceManager svc_mgr;
    auto cond_svc = std::make_unique<ConditionsService>();
    svc_mgr.add_service(std::move(cond_svc));

    auto sc = svc_mgr.initialize_all();
    EXPECT_TRUE(sc.is_success());
}

TEST(ConditionsServiceTest, RunNumber) {
    ConditionsService svc;
    svc.initialize();

    EXPECT_EQ(svc.run_number(), 0);

    svc.set_run_number(12345);
    EXPECT_EQ(svc.run_number(), 12345);
}

TEST(ConditionsServiceTest, SetAndGetValue) {
    ConditionsService svc;
    svc.initialize();
    svc.set_run_number(100);

    IntervalOfValidity iov{50, 150};
    svc.set_value("test_param", 42.5, iov);

    double value = svc.get_value<double>("test_param", 0.0);
    EXPECT_DOUBLE_EQ(value, 42.5);
}

TEST(ConditionsServiceTest, IOVCheck) {
    ConditionsService svc;
    svc.initialize();

    IntervalOfValidity iov{100, 200};
    svc.set_value("param", 123, iov);

    // Run inside IOV
    svc.set_run_number(150);
    int value = svc.get_value<int>("param", 0);
    EXPECT_EQ(value, 123);

    // Run outside IOV
    svc.set_run_number(50);
    value = svc.get_value<int>("param", 999);
    EXPECT_EQ(value, 999);  // Should return default
}
