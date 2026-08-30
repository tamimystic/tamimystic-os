#pragma once

#include "os_robotics_types.h"
#include "os_kinematics.h"
#include "os_servo_controller.h"
#include <string>
#include <vector>

namespace TamimysticOS {

class RobotController {
public:
    static RobotController& getInstance();

    // Initialize robotics subsystem, kinematics, drivers, and background task
    void init();

    // Set active robot topology / mode
    void setMode(RobotMode mode);
    RobotMode getMode() const;

    // Command rover velocity (Twist: linear vx, strafe vy, angular omega)
    void setTwist(float vx, float vy, float omega);

    // Direct differential drive speed (-100 to +100)
    void setDifferentialSpeed(int left_speed, int right_speed);

    // Command robotic arm by joint angles (0 - 180 deg)
    void setArmJoints(const ArmJoints& joints);

    // Command robotic arm by Cartesian target coordinates (X, Y, Z in cm)
    bool setArmTargetIK(const ArmPose& target);

    // Trigger Emergency Stop (immediate full brake)
    void emergencyStop();

    // Resume from emergency stop
    void resume();

    // Get live telemetry struct and JSON
    RoboticsTelemetry getTelemetry() const;
    std::string getTelemetryJson();

    // Safety and obstacle distance update
    void updateObstacleDistance(float distance_cm);

    // Real-time control loop iteration (50Hz)
    void processControlLoop();

private:
    RobotController() = default;
    ~RobotController() = default;

    void applyWheelOutputs(const WheelSpeeds& speeds);

    RobotMode current_mode = RobotMode::DIFFERENTIAL_ROVER;
    TwistVelocity current_twist;
    WheelSpeeds current_wheels;
    ArmJoints current_joints;
    ArmPose current_pose;

    float obstacle_distance_cm = 999.0f;
    bool is_e_stopped = false;
    bool is_braking_for_obstacle = false;
    uint32_t last_cmd_timestamp_ms = 0;
};

// Backward-compatible MotorDriver wrapper
class MotorDriver {
public:
    static MotorDriver& getInstance();
    void init() { RobotController::getInstance().init(); }
    void setSpeed(int left, int right) { RobotController::getInstance().setDifferentialSpeed(left, right); }
};

} // namespace TamimysticOS
