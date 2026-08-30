#pragma once

#include "os_robotics_types.h"
#include <cmath>

namespace TamimysticOS {

class KinematicsEngine {
public:
    static KinematicsEngine& getInstance();

    // 1. Differential Drive Kinematics (Rover 2WD/4WD)
    WheelSpeeds computeDifferential(float linear_speed, float angular_speed);

    // 2. Mecanum / Holonomic Drive Kinematics (4WD)
    WheelSpeeds computeMecanum(float vx, float vy, float omega);

    // 3. Robotic Arm Kinematics (3-6 DOF)
    bool solveInverseKinematics(const ArmPose& target_pose, ArmJoints& out_joints);
    ArmPose solveForwardKinematics(const ArmJoints& joints);

    // Set arm segment lengths (cm)
    void setArmLinkLengths(float base_h, float upper_arm_l, float forearm_l, float wrist_l);

private:
    KinematicsEngine();
    ~KinematicsEngine() = default;

    // Default arm geometry parameters (in cm)
    float L0_base = 6.0f;     // Base height to shoulder joint
    float L1_upper = 12.0f;   // Upper arm length (Shoulder to Elbow)
    float L2_fore = 12.0f;    // Forearm length (Elbow to Wrist)
    float L3_wrist = 8.0f;    // Wrist to Gripper tip
};

} // namespace TamimysticOS
