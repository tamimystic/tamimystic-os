#include "os_kinematics.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace TamimysticOS {

static inline float rad2deg(float rad) { return rad * 180.0f / (float)M_PI; }
static inline float deg2rad(float deg) { return deg * (float)M_PI / 180.0f; }

KinematicsEngine& KinematicsEngine::getInstance() {
    static KinematicsEngine instance;
    return instance;
}

KinematicsEngine::KinematicsEngine() {
    setArmLinkLengths(6.0f, 12.0f, 12.0f, 8.0f);
}

void KinematicsEngine::setArmLinkLengths(float base_h, float upper_arm_l, float forearm_l, float wrist_l) {
    L0_base = base_h;
    L1_upper = upper_arm_l;
    L2_fore = forearm_l;
    L3_wrist = wrist_l;
}

WheelSpeeds KinematicsEngine::computeDifferential(float linear_speed, float angular_speed) {
    WheelSpeeds ws;
    // Standard differential drive mixer
    float left = linear_speed - angular_speed;
    float right = linear_speed + angular_speed;

    // Normalization to maintain ratio if exceeding 100%
    float max_mag = std::max(std::abs(left), std::abs(right));
    if (max_mag > 100.0f) {
        left = (left / max_mag) * 100.0f;
        right = (right / max_mag) * 100.0f;
    }

    ws.front_left = left;
    ws.rear_left = left;
    ws.front_right = right;
    ws.rear_right = right;
    return ws;
}

WheelSpeeds KinematicsEngine::computeMecanum(float vx, float vy, float omega) {
    WheelSpeeds ws;
    // Standard 4-wheel mecanum kinematics
    ws.front_left  = vx + vy + omega;
    ws.front_right = vx - vy - omega;
    ws.rear_left   = vx - vy + omega;
    ws.rear_right  = vx + vy - omega;

    // Normalize if any wheel speed exceeds 100%
    float max_mag = std::max({std::abs(ws.front_left), std::abs(ws.front_right), 
                              std::abs(ws.rear_left), std::abs(ws.rear_right)});
    if (max_mag > 100.0f) {
        ws.front_left  = (ws.front_left / max_mag) * 100.0f;
        ws.front_right = (ws.front_right / max_mag) * 100.0f;
        ws.rear_left   = (ws.rear_left / max_mag) * 100.0f;
        ws.rear_right  = (ws.rear_right / max_mag) * 100.0f;
    }
    return ws;
}

bool KinematicsEngine::solveInverseKinematics(const ArmPose& target, ArmJoints& out_joints) {
    // 1. Base Yaw Calculation
    float base_angle_rad = std::atan2(target.y, target.x);
    float base_yaw_deg = 90.0f + rad2deg(base_angle_rad);
    if (base_yaw_deg < 0.0f || base_yaw_deg > 180.0f) {
        return false; // Out of base rotation workspace
    }

    // 2. Planar Projection in R-Z plane
    float r_total = std::sqrt(target.x * target.x + target.y * target.y);
    float phi_rad = deg2rad(target.pitch);

    // Target wrist center position
    float r_wrist = r_total - L3_wrist * std::cos(phi_rad);
    float z_wrist = target.z - L3_wrist * std::sin(phi_rad) - L0_base;

    float D_sq = r_wrist * r_wrist + z_wrist * z_wrist;
    float D = std::sqrt(D_sq);

    // Reachability check
    if (D > (L1_upper + L2_fore) || D < std::abs(L1_upper - L2_fore)) {
        return false; // Target point unreachable
    }

    // 3. Law of Cosines for Elbow Angle
    float cos_elbow = (L1_upper * L1_upper + L2_fore * L2_fore - D_sq) / (2.0f * L1_upper * L2_fore);
    cos_elbow = std::clamp(cos_elbow, -1.0f, 1.0f);
    float elbow_internal_rad = std::acos(cos_elbow);
    float elbow_pitch_deg = rad2deg(M_PI - elbow_internal_rad);

    // 4. Shoulder Angle
    float alpha = std::atan2(z_wrist, r_wrist);
    float cos_beta = (L1_upper * L1_upper + D_sq - L2_fore * L2_fore) / (2.0f * L1_upper * D);
    cos_beta = std::clamp(cos_beta, -1.0f, 1.0f);
    float beta = std::acos(cos_beta);
    float shoulder_pitch_deg = rad2deg(alpha + beta);

    // 5. Wrist Pitch Angle
    float wrist_pitch_deg = 90.0f + (target.pitch - (shoulder_pitch_deg + elbow_pitch_deg - 180.0f));

    // Clamp and assign joint values (0 - 180 deg servo range)
    out_joints.base_yaw = std::clamp(base_yaw_deg, 0.0f, 180.0f);
    out_joints.shoulder_pitch = std::clamp(shoulder_pitch_deg, 0.0f, 180.0f);
    out_joints.elbow_pitch = std::clamp(elbow_pitch_deg, 0.0f, 180.0f);
    out_joints.wrist_pitch = std::clamp(wrist_pitch_deg, 0.0f, 180.0f);
    out_joints.wrist_roll = 90.0f; // Default center
    out_joints.gripper = std::clamp(target.gripper, 0.0f, 100.0f);

    return true;
}

ArmPose KinematicsEngine::solveForwardKinematics(const ArmJoints& joints) {
    ArmPose pose;
    float base_yaw_rad = deg2rad(joints.base_yaw - 90.0f);
    float shoulder_rad = deg2rad(joints.shoulder_pitch);
    float elbow_rad = deg2rad(joints.elbow_pitch);
    float wrist_rad = deg2rad(joints.wrist_pitch - 90.0f);

    // Relative joint angles in 2D plane
    float theta1 = shoulder_rad;
    float theta2 = theta1 + elbow_rad - (float)M_PI;
    float theta3 = theta2 + wrist_rad;

    float r = L1_upper * std::cos(theta1) + L2_fore * std::cos(theta2) + L3_wrist * std::cos(theta3);
    float z = L0_base + L1_upper * std::sin(theta1) + L2_fore * std::sin(theta2) + L3_wrist * std::sin(theta3);

    pose.x = r * std::cos(base_yaw_rad);
    pose.y = r * std::sin(base_yaw_rad);
    pose.z = z;
    pose.pitch = rad2deg(theta3);
    pose.gripper = joints.gripper;

    return pose;
}

} // namespace TamimysticOS
