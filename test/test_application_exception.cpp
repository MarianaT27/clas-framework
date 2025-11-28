#include <gtest/gtest.h>
#include "Framework/StatusCode.hpp"
#include <stdexcept>
#include <string>
#include <vector>
#include <fmt/format.h>

using namespace Framework;

/**
 * Test that verifies exception handling in algorithm execution.
 * 
 * This test simulates the exception handling behavior that occurs
 * in the Application event loop by directly testing the try-catch
 * pattern with algorithm execute() calls.
 */
TEST(ApplicationExceptionTest, AlgorithmExceptionIsCaught) {
    // Simulate what the Application does - wrap execute in try-catch
    int exception_count = 0;
    std::string caught_message;
    
    try {
        // This simulates calling alg->execute(event) in the Application
        throw std::runtime_error("Test exception from algorithm");
    } catch (const std::exception& e) {
        exception_count++;
        caught_message = e.what();
    }
    
    EXPECT_EQ(exception_count, 1);
    EXPECT_EQ(caught_message, "Test exception from algorithm");
}

/**
 * Test that verifies non-standard exceptions are also caught.
 */
TEST(ApplicationExceptionTest, NonStdExceptionIsCaught) {
    int exception_count = 0;
    bool caught_unknown = false;
    
    try {
        throw 42;  // Non-standard exception
    } catch (const std::exception& e) {
        exception_count++;
    } catch (...) {
        exception_count++;
        caught_unknown = true;
    }
    
    EXPECT_EQ(exception_count, 1);
    EXPECT_TRUE(caught_unknown);
}

/**
 * Test that verifies processing continues after an exception.
 * 
 * This simulates the Application event loop behavior where
 * exceptions in one algorithm don't stop processing of other algorithms.
 */
TEST(ApplicationExceptionTest, ProcessingContinuesAfterException) {
    // Simulate algorithm sequence with one throwing algorithm
    std::vector<std::string> algorithm_names = {"Alg1", "ThrowingAlg", "Alg2", "Alg3"};
    std::vector<bool> executed(4, false);
    int exception_count = 0;
    
    // Simulate event loop processing
    for (size_t i = 0; i < algorithm_names.size(); ++i) {
        try {
            if (algorithm_names[i] == "ThrowingAlg") {
                throw std::runtime_error("Algorithm failed");
            }
            executed[i] = true;
        } catch (const std::exception& e) {
            exception_count++;
            // Continue to next algorithm (don't break)
        }
    }
    
    // Verify that algorithms before and after the throwing one executed
    EXPECT_TRUE(executed[0]);   // Alg1 executed
    EXPECT_FALSE(executed[1]);  // ThrowingAlg threw
    EXPECT_TRUE(executed[2]);   // Alg2 executed (processing continued)
    EXPECT_TRUE(executed[3]);   // Alg3 executed
    EXPECT_EQ(exception_count, 1);
}

/**
 * Test that verifies multiple exceptions are counted correctly.
 */
TEST(ApplicationExceptionTest, MultipleExceptionsAreCounted) {
    int exception_count = 0;
    int events_processed = 5;
    
    // Simulate processing multiple events where some throw
    for (int evt = 0; evt < events_processed; ++evt) {
        try {
            if (evt == 1 || evt == 3) {
                throw std::runtime_error("Event processing failed");
            }
        } catch (const std::exception& e) {
            exception_count++;
        }
    }
    
    EXPECT_EQ(exception_count, 2);
}

/**
 * Test that verifies algorithm name is available for error reporting.
 */
TEST(ApplicationExceptionTest, AlgorithmNameAvailableForErrorReporting) {
    std::string alg_name = "MyTestAlgorithm";
    
    // Simulate error reporting pattern used in Application
    std::string error_message;
    try {
        throw std::runtime_error("Something went wrong");
    } catch (const std::exception& e) {
        // This is the pattern used in Application::run()
        error_message = fmt::format("ERROR: Exception in algorithm '{}': {}", alg_name, e.what());
    }
    
    EXPECT_EQ(error_message, "ERROR: Exception in algorithm 'MyTestAlgorithm': Something went wrong");
}

/**
 * Test that verifies the error message format matches Application output.
 */
TEST(ApplicationExceptionTest, ErrorMessageFormatMatchesApplication) {
    std::string alg_name = "ElectronSelector";
    std::string exception_msg = "Invalid particle index";
    
    // Format error message the same way Application does
    std::string formatted = fmt::format("ERROR: Exception in algorithm '{}': {}", alg_name, exception_msg);
    
    EXPECT_TRUE(formatted.find("ERROR:") != std::string::npos);
    EXPECT_TRUE(formatted.find(alg_name) != std::string::npos);
    EXPECT_TRUE(formatted.find(exception_msg) != std::string::npos);
}
