#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TamimysticOS {

enum class RobotMode {
    DIFFERENTIAL_ROVER = 0, // 2WD / 4WD Differential Drive
    MECANUM_4WD,            // 4-Wheel Holonomic Mecanum Drive
    ROBOTIC_ARM,            // 3 to 6-DOF Robotic Arm with IK
    BALANCE_BOT             // Two-wheeled Self-Balancing Robot
};

inline const char* robotModeToString(RobotMode mode) {
    switch (mode) {
        case RobotMode::DIFFERENTIAL_ROVER: return "Differential Rover (2WD/4WD)";
        case RobotMode::MECANUM_4WD:        return "Mecanum Holonomic 4WD";
        case RobotMode::ROBOTIC_ARM:        return "Robotic Arm (IK/FK)";
        case RobotMode::BALANCE_BOT:        return "Self-Balancing Robot";
        default:                            return "Unknown Mode";
    }
}

struct TwistVelocity {
    float vx = 0.0f;    // Forward/Backward (-100% to +100%)
    float vy = 0.0f;    // Strafe Left/Right (-100% to +100% for Mecanum)
    float omega = 0.0f; // Angular Rotation (-100% to +100%)
};

struct WheelSpeeds {
    float front_left = 0.0f;
    float front_right = 0.0f;
    float rear_left = 0.0f;
    float rear_right = 0.0f;
};

struct ArmJoints {
    float base_yaw = 90.0f;      // Joint 1: Base Rotation (0 - 180 deg)
    float shoulder_pitch = 90.0f;// Joint 2: Shoulder (0 - 180 deg)
    float elbow_pitch = 90.0f;   // Joint 3: Elbow (0 - 180 deg)
    float wrist_pitch = 90.0f;   // Joint 4: Wrist Pitch (0 - 180 deg)
    float wrist_roll = 90.0f;    // Joint 5: Wrist Roll (0 - 180 deg)
    float gripper = 0.0f;        // Joint 6: Gripper (0 = Open, 100 = Closed)
};

struct ArmPose {
    float x = 15.0f;     // Forward distance in cm
    float y = 0.0f;      // Lateral distance in cm
    float z = 10.0f;     // Height in cm
    float pitch = 0.0f;  // End-effector pitch angle in deg
    float gripper = 0.0f;// Gripper state (0 - 100%)
};

struct RoboticsTelemetry {
    RobotMode mode = RobotMode::DIFFERENTIAL_ROVER;
    TwistVelocity velocity;
    WheelSpeeds wheels;
    ArmJoints joints;
    ArmPose pose;
    float obstacle_distance_cm = 999.0f;
    bool emergency_stop = false;
    bool obstacle_braking = false;
    uint32_t active_time_ms = 0;
};

} // namespace TamimysticOS
