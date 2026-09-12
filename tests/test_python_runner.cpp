#include "test_framework.h"
#include "os_python_runner.h"
#include "os_robotics.h"

using namespace TamimysticOS;

void test_python_variable_and_math() {
    auto& py = PythonRunner::getInstance();
    py.init();

    std::string script = 
        "x = 10\n"
        "y = 20\n"
        "z = x + y\n"
        "print('Result is:', z)\n";

    ScriptExecutionResult res = py.eval(script);
    TEST_ASSERT(res.success, "Script evaluation should succeed");
    TEST_ASSERT(res.stdout_output.find("Result is: 30") != std::string::npos, "Math result should be 30");
}

void test_python_for_loop_block() {
    auto& py = PythonRunner::getInstance();
    py.init();

    std::string script = 
        "count = 0\n"
        "for i in range(5):\n"
        "  count = count + 1\n"
        "print('Final count:', count)\n";

    ScriptExecutionResult res = py.eval(script);
    TEST_ASSERT(res.success, "For loop script execution should succeed");
    TEST_ASSERT(res.stdout_output.find("Final count: 5") != std::string::npos, "Loop should iterate 5 times");
}

void test_python_if_else_branching() {
    auto& py = PythonRunner::getInstance();
    py.init();

    std::string script = 
        "dist = 12\n"
        "if dist < 15:\n"
        "  print('Obstacle detected')\n"
        "else:\n"
        "  print('Path clear')\n";

    ScriptExecutionResult res = py.eval(script);
    TEST_ASSERT(res.success, "If-else script should execute cleanly");
    TEST_ASSERT(res.stdout_output.find("Obstacle detected") != std::string::npos, "If condition should evaluate to true");
}

void run_python_runner_test_suite() {
    RUN_TEST_SUITE("MicroPython Variable & Math Evaluation", test_python_variable_and_math);
    RUN_TEST_SUITE("MicroPython Multi-Line For Loop Control Flow", test_python_for_loop_block);
    RUN_TEST_SUITE("MicroPython If-Else Conditional Branching", test_python_if_else_branching);
}
