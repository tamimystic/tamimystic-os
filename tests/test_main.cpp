#include "test_framework.h"
#include <iostream>
#include <iomanip>

// Declarations of individual test suites
void run_kinematics_test_suite();
void run_arm_ik_test_suite();
void run_slam_nav_test_suite();
void run_espnow_swarm_test_suite();
void run_ble_test_suite();
void run_audio_dsp_test_suite();
void run_event_bus_test_suite();

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    std::cout << "\n=======================================================\n";
    std::cout << "     TAMIMYSTIC OS - AUTOMATED CTEST UNIT TEST SUITE    \n";
    std::cout << "=======================================================\n\n";

    auto start_all = std::chrono::high_resolution_clock::now();

    // Execute all component test suites
    run_kinematics_test_suite();
    run_arm_ik_test_suite();
    run_slam_nav_test_suite();
    run_espnow_swarm_test_suite();
    run_ble_test_suite();
    run_audio_dsp_test_suite();
    run_event_bus_test_suite();

    auto end_all = std::chrono::high_resolution_clock::now();
    double total_ms = std::chrono::duration<double, std::milli>(end_all - start_all).count();

    const auto& stats = TamimysticTest::getStats();

    std::cout << "\n-------------------------------------------------------\n";
    std::cout << "                   TEST SUMMARY RESULTS                \n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << "  Test Suites Run:       " << stats.tests_run << "\n";
    std::cout << "  Test Suites Passed:    " << stats.suites_passed << "\n";
    std::cout << "  Test Suites Failed:    " << stats.suites_failed << "\n";
    std::cout << "  Total Assertions:      " << (stats.assertions_passed + stats.assertions_failed) << "\n";
    std::cout << "  Assertions Passed:     " << stats.assertions_passed << "\n";
    std::cout << "  Assertions Failed:     " << stats.assertions_failed << "\n";
    std::cout << "  Total Execution Time:  " << std::fixed << std::setprecision(2) << total_ms << " ms\n";
    std::cout << "-------------------------------------------------------\n";

    if (stats.assertions_failed == 0 && stats.suites_failed == 0) {
        std::cout << "\n >>> ALL UNIT TESTS COMPLETED SUCCESSFULLY! [100% PASS] <<<\n\n";
        return 0;
    } else {
        std::cout << "\n >>> REGRESSION DETECTED: SOME TESTS FAILED! <<<\n\n";
        return 1;
    }
}
