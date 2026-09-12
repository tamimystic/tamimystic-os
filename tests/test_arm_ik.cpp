#include "test_framework.h"
#include "os_kinematics.h"

using namespace TamimysticOS;

void test_arm_ik_convergence() {
    auto& ke = KinematicsEngine::getInstance();
    ke.setArmLinkLengths(6.0f, 12.0f, 12.0f, 8.0f);

    // Test 1: Standard nominal reachable pose (X=16.0cm, Y=0.0cm, Z=12.0cm, Pitch=0 deg)
    ArmPose target1;
    target1.x = 16.0f;
    target1.y = 0.0f;
    target1.z = 12.0f;
    target1.pitch = 0.0f;
    target1.gripper = 50.0f;

    ArmJoints computed_joints;
    bool ik_success = ke.solveInverseKinematics(target1, computed_joints);
    TEST_ASSERT(ik_success, "IK solver should succeed for reachable nominal coordinate");

    // Verify joint limits (0 to 180 degrees)
    TEST_ASSERT(computed_joints.base_yaw >= 0.0f && computed_joints.base_yaw <= 180.0f, "Base yaw within limits");
    TEST_ASSERT(computed_joints.shoulder_pitch >= 0.0f && computed_joints.shoulder_pitch <= 180.0f, "Shoulder within limits");
    TEST_ASSERT(computed_joints.elbow_pitch >= 0.0f && computed_joints.elbow_pitch <= 180.0f, "Elbow within limits");
    TEST_ASSERT(computed_joints.wrist_pitch >= 0.0f && computed_joints.wrist_pitch <= 180.0f, "Wrist within limits");

    // Forward Kinematics round-trip consistency test
    ArmPose recovered_pose = ke.solveForwardKinematics(computed_joints);
    TEST_ASSERT_FLOAT_NEAR(recovered_pose.x, target1.x, 0.5f, "FK/IK round-trip X coordinate recovery");
    TEST_ASSERT_FLOAT_NEAR(recovered_pose.y, target1.y, 0.5f, "FK/IK round-trip Y coordinate recovery");
    TEST_ASSERT_FLOAT_NEAR(recovered_pose.z, target1.z, 0.5f, "FK/IK round-trip Z coordinate recovery");
}

void test_arm_unreachable_boundary() {
    auto& ke = KinematicsEngine::getInstance();

    // Far unreachable coordinate (X=100.0cm, beyond maximum arm reach of 38cm)
    ArmPose target_far;
    target_far.x = 100.0f;
    target_far.y = 50.0f;
    target_far.z = 20.0f;
    target_far.pitch = 0.0f;

    ArmJoints joints;
    bool ok = ke.solveInverseKinematics(target_far, joints);
    TEST_ASSERT(!ok, "IK solver must reject out-of-reach target point");
}

void run_arm_ik_test_suite() {
    RUN_TEST_SUITE("Robotic Arm 6-DOF Inverse & Forward Kinematics", test_arm_ik_convergence);
    RUN_TEST_SUITE("Robotic Arm Workspace Boundary & Singularity Rejection", test_arm_unreachable_boundary);
}
