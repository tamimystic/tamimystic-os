#include "test_framework.h"
#include "os_kinematics.h"

using namespace TamimysticOS;

void test_differential_drive() {
    auto& ke = KinematicsEngine::getInstance();

    // Pure forward motion (vx=50, omega=0)
    WheelSpeeds ws1 = ke.computeDifferential(50.0f, 0.0f);
    TEST_ASSERT_FLOAT_NEAR(ws1.front_left, 50.0f, 0.001f, "Diff pure forward left wheel");
    TEST_ASSERT_FLOAT_NEAR(ws1.front_right, 50.0f, 0.001f, "Diff pure forward right wheel");
    TEST_ASSERT_FLOAT_NEAR(ws1.rear_left, 50.0f, 0.001f, "Diff pure forward rear left");
    TEST_ASSERT_FLOAT_NEAR(ws1.rear_right, 50.0f, 0.001f, "Diff pure forward rear right");

    // Pure rotation (vx=0, omega=30)
    WheelSpeeds ws2 = ke.computeDifferential(0.0f, 30.0f);
    TEST_ASSERT_FLOAT_NEAR(ws2.front_left, -30.0f, 0.001f, "Diff pure spin left wheel");
    TEST_ASSERT_FLOAT_NEAR(ws2.front_right, 30.0f, 0.001f, "Diff pure spin right wheel");

    // Clamping normalization (vx=80, omega=60 -> sum=140 > 100)
    WheelSpeeds ws3 = ke.computeDifferential(80.0f, 60.0f);
    TEST_ASSERT(std::abs(ws3.front_right) <= 100.001f, "Diff speed clamped to 100%");
    TEST_ASSERT(std::abs(ws3.front_left) <= 100.001f, "Diff speed clamped to 100%");
}

void test_mecanum_drive() {
    auto& ke = KinematicsEngine::getInstance();

    // Pure strafe right (vx=0, vy=50, omega=0)
    // ws.FL = vx + vy + omega = 50
    // ws.FR = vx - vy - omega = -50
    // ws.RL = vx - vy + omega = -50
    // ws.RR = vx + vy - omega = 50
    WheelSpeeds ws1 = ke.computeMecanum(0.0f, 50.0f, 0.0f);
    TEST_ASSERT_FLOAT_NEAR(ws1.front_left, 50.0f, 0.001f, "Mecanum strafe FL");
    TEST_ASSERT_FLOAT_NEAR(ws1.front_right, -50.0f, 0.001f, "Mecanum strafe FR");
    TEST_ASSERT_FLOAT_NEAR(ws1.rear_left, -50.0f, 0.001f, "Mecanum strafe RL");
    TEST_ASSERT_FLOAT_NEAR(ws1.rear_right, 50.0f, 0.001f, "Mecanum strafe RR");

    // Pure diagonal translation (vx=40, vy=40, omega=0)
    WheelSpeeds ws2 = ke.computeMecanum(40.0f, 40.0f, 0.0f);
    TEST_ASSERT_FLOAT_NEAR(ws2.front_left, 80.0f, 0.001f, "Mecanum diagonal FL");
    TEST_ASSERT_FLOAT_NEAR(ws2.front_right, 0.0f, 0.001f, "Mecanum diagonal FR");
    TEST_ASSERT_FLOAT_NEAR(ws2.rear_left, 0.0f, 0.001f, "Mecanum diagonal RL");
    TEST_ASSERT_FLOAT_NEAR(ws2.rear_right, 80.0f, 0.001f, "Mecanum diagonal RR");
}

void run_kinematics_test_suite() {
    RUN_TEST_SUITE("Differential Drive Kinematics", test_differential_drive);
    RUN_TEST_SUITE("Mecanum 4WD Omnidirectional Kinematics", test_mecanum_drive);
}
