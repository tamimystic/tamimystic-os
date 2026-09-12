#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

namespace TamimysticTest {

struct TestStats {
    int tests_run = 0;
    int assertions_passed = 0;
    int assertions_failed = 0;
    int suites_passed = 0;
    int suites_failed = 0;
};

inline TestStats& getStats() {
    static TestStats stats;
    return stats;
}

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            TamimysticTest::getStats().assertions_passed++; \
        } else { \
            TamimysticTest::getStats().assertions_failed++; \
            std::cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << " -> " << message << std::endl; \
        } \
    } while (0)

#define TEST_ASSERT_FLOAT_NEAR(a, b, epsilon, message) \
    do { \
        if (std::abs((a) - (b)) <= (epsilon)) { \
            TamimysticTest::getStats().assertions_passed++; \
        } else { \
            TamimysticTest::getStats().assertions_failed++; \
            std::cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << " -> " << message \
                      << " (Expected: " << (b) << ", Actual: " << (a) << ", Diff: " << std::abs((a) - (b)) << ")" << std::endl; \
        } \
    } while (0)

#define TEST_ASSERT_EQ(a, b, message) \
    do { \
        if ((a) == (b)) { \
            TamimysticTest::getStats().assertions_passed++; \
        } else { \
            TamimysticTest::getStats().assertions_failed++; \
            std::cerr << "  [FAIL] " << __FILE__ << ":" << __LINE__ << " -> " << message \
                      << " (Expected: " << (b) << ", Actual: " << (a) << ")" << std::endl; \
        } \
    } while (0)

#define RUN_TEST_SUITE(suite_name, func) \
    do { \
        std::cout << "[RUNNING TEST SUITE] " << suite_name << "..." << std::endl; \
        TamimysticTest::getStats().tests_run++; \
        int initial_failures = TamimysticTest::getStats().assertions_failed; \
        auto start = std::chrono::high_resolution_clock::now(); \
        func(); \
        auto end = std::chrono::high_resolution_clock::now(); \
        double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count(); \
        if (TamimysticTest::getStats().assertions_failed == initial_failures) { \
            TamimysticTest::getStats().suites_passed++; \
            std::cout << "  [PASS] " << suite_name << " (" << elapsed_us << " us)" << std::endl; \
        } else { \
            TamimysticTest::getStats().suites_failed++; \
            std::cout << "  [FAIL] " << suite_name << " had assertion errors!" << std::endl; \
        } \
    } while (0)

} // namespace TamimysticTest
