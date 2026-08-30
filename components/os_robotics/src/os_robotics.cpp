#include "os_robotics.h"
#include "os_hal_uart.h"
#include "os_hal_pwm.h"
#include "os_hal_gpio.h"
#include "os_pin_matrix.h"
#include "os_scheduler.h"
#include "os_event_bus.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

#define PWM_CHANNEL_LEFT 0
#define PWM_CHANNEL_RIGHT 1

namespace TamimysticOS {

MotorDriver& MotorDriver::getInstance() {
    static MotorDriver instance;
    return instance;
}

RobotController& RobotController::getInstance() {
    static RobotController instance;
    return instance;
}

void RobotController::init() {
    hal_uart_print("[ROBOTICS] Initializing Universal Robot Brain on Core 1...\n");

    auto& pm = PinMatrixManager::getInstance();
    int in1 = pm.getPin(PinFunction::MOTOR_L_IN1);
    int in2 = pm.getPin(PinFunction::MOTOR_L_IN2);
    int in3 = pm.getPin(PinFunction::MOTOR_R_IN3);
    int in4 = pm.getPin(PinFunction::MOTOR_R_IN4);
    int pwmL = pm.getPin(PinFunction::MOTOR_L_PWM);
    int pwmR = pm.getPin(PinFunction::MOTOR_R_PWM);

    // Initialize DC Motor GPIOs
    if (in1 >= 0) hal_gpio_set_direction(in1, HAL_GPIO_MODE_OUTPUT);
    if (in2 >= 0) hal_gpio_set_direction(in2, HAL_GPIO_MODE_OUTPUT);
    if (in3 >= 0) hal_gpio_set_direction(in3, HAL_GPIO_MODE_OUTPUT);
    if (in4 >= 0) hal_gpio_set_direction(in4, HAL_GPIO_MODE_OUTPUT);

    // Initialize PWM channels (10 kHz, 8-bit resolution)
    if (pwmL >= 0) hal_pwm_init(pwmL, PWM_CHANNEL_LEFT, 10000, 8);
    if (pwmR >= 0) hal_pwm_init(pwmR, PWM_CHANNEL_RIGHT, 10000, 8);

    // Initialize Servo Subsystem
    ServoController::getInstance().init();

    // Default arm home pose
    current_pose = {15.0f, 0.0f, 10.0f, 0.0f, 0.0f};
    KinematicsEngine::getInstance().solveInverseKinematics(current_pose, current_joints);
    ServoController::getInstance().applyArmJoints(current_joints);

    // Spawn 50Hz Real-Time Robotics Task on Core 1
    OSScheduler::getInstance().createTask("robot_ctrl_loop", 4096, 3, CORE_1, [this]() {
        while (true) {
            this->processControlLoop();
            OSScheduler::getInstance().delay(20); // 50 Hz loop
        }
    });

    hal_uart_print("[ROBOTICS] Universal Kinematics & Control Engine Active.\n");
}

void RobotController::setMode(RobotMode mode) {
    current_mode = mode;
    emergencyStop(); // Reset motion on mode switch
    hal_uart_print(("[ROBOTICS] Switched Mode to: " + std::string(robotModeToString(mode)) + "\n").c_str());
}

RobotMode RobotController::getMode() const {
    return current_mode;
}

void RobotController::setTwist(float vx, float vy, float omega) {
    if (is_e_stopped) return;

    current_twist.vx = std::clamp(vx, -100.0f, 100.0f);
    current_twist.vy = std::clamp(vy, -100.0f, 100.0f);
    current_twist.omega = std::clamp(omega, -100.0f, 100.0f);

    // Calculate wheel speeds based on active topology
    if (current_mode == RobotMode::MECANUM_4WD) {
        current_wheels = KinematicsEngine::getInstance().computeMecanum(current_twist.vx, current_twist.vy, current_twist.omega);
    } else {
        current_wheels = KinematicsEngine::getInstance().computeDifferential(current_twist.vx, current_twist.omega);
    }
}

void RobotController::setDifferentialSpeed(int left_speed, int right_speed) {
    if (is_e_stopped) return;

    current_twist.vx = (float)(left_speed + right_speed) / 2.0f;
    current_twist.omega = (float)(right_speed - left_speed) / 2.0f;
    current_wheels.front_left = (float)left_speed;
    current_wheels.rear_left = (float)left_speed;
    current_wheels.front_right = (float)right_speed;
    current_wheels.rear_right = (float)right_speed;
}

void RobotController::setArmJoints(const ArmJoints& joints) {
    current_joints = joints;
    current_pose = KinematicsEngine::getInstance().solveForwardKinematics(joints);
    ServoController::getInstance().applyArmJoints(current_joints);
}

bool RobotController::setArmTargetIK(const ArmPose& target) {
    ArmJoints calculated_joints;
    bool solvable = KinematicsEngine::getInstance().solveInverseKinematics(target, calculated_joints);
    if (solvable) {
        current_pose = target;
        current_joints = calculated_joints;
        ServoController::getInstance().applyArmJoints(current_joints);
        return true;
    }
    return false;
}

void RobotController::emergencyStop() {
    is_e_stopped = true;
    current_twist = {0.0f, 0.0f, 0.0f};
    current_wheels = {0.0f, 0.0f, 0.0f, 0.0f};
    applyWheelOutputs(current_wheels);
    hal_uart_print("[ROBOTICS:SAFETY] Emergency Stop Engaged!\n");
}

void RobotController::resume() {
    is_e_stopped = false;
    is_braking_for_obstacle = false;
    hal_uart_print("[ROBOTICS:SAFETY] Emergency Stop Released.\n");
}

void RobotController::updateObstacleDistance(float distance_cm) {
    obstacle_distance_cm = distance_cm;
}

void RobotController::processControlLoop() {
    if (is_e_stopped) {
        applyWheelOutputs({0.0f, 0.0f, 0.0f, 0.0f});
        return;
    }

    // Safety Bumper: Prevent driving forward if obstacle is < 15cm
    if (current_twist.vx > 0.0f && obstacle_distance_cm < 15.0f) {
        is_braking_for_obstacle = true;
        current_wheels.front_left = 0.0f;
        current_wheels.front_right = 0.0f;
        current_wheels.rear_left = 0.0f;
        current_wheels.rear_right = 0.0f;
    } else {
        is_braking_for_obstacle = false;
    }

    applyWheelOutputs(current_wheels);
}

void RobotController::applyWheelOutputs(const WheelSpeeds& ws) {
    auto& pm = PinMatrixManager::getInstance();
    int in1 = pm.getPin(PinFunction::MOTOR_L_IN1);
    int in2 = pm.getPin(PinFunction::MOTOR_L_IN2);
    int in3 = pm.getPin(PinFunction::MOTOR_R_IN3);
    int in4 = pm.getPin(PinFunction::MOTOR_R_IN4);

    float left_spd = ws.front_left;
    float right_spd = ws.front_right;

    // Left Motor H-Bridge
    if (left_spd >= 0) {
        if (in1 >= 0) hal_gpio_set_level(in1, 1);
        if (in2 >= 0) hal_gpio_set_level(in2, 0);
    } else {
        if (in1 >= 0) hal_gpio_set_level(in1, 0);
        if (in2 >= 0) hal_gpio_set_level(in2, 1);
        left_spd = -left_spd;
    }

    // Right Motor H-Bridge
    if (right_spd >= 0) {
        if (in3 >= 0) hal_gpio_set_level(in3, 1);
        if (in4 >= 0) hal_gpio_set_level(in4, 0);
    } else {
        if (in3 >= 0) hal_gpio_set_level(in3, 0);
        if (in4 >= 0) hal_gpio_set_level(in4, 1);
        right_spd = -right_spd;
    }

    hal_pwm_set_duty(PWM_CHANNEL_LEFT, (int)std::clamp(left_spd, 0.0f, 100.0f));
    hal_pwm_set_duty(PWM_CHANNEL_RIGHT, (int)std::clamp(right_spd, 0.0f, 100.0f));
}

RoboticsTelemetry RobotController::getTelemetry() const {
    RoboticsTelemetry tel;
    tel.mode = current_mode;
    tel.velocity = current_twist;
    tel.wheels = current_wheels;
    tel.joints = current_joints;
    tel.pose = current_pose;
    tel.obstacle_distance_cm = obstacle_distance_cm;
    tel.emergency_stop = is_e_stopped;
    tel.obstacle_braking = is_braking_for_obstacle;
    return tel;
}

std::string RobotController::getTelemetryJson() {
    std::stringstream ss;
    ss << "{"
       << "\"status\":\"ok\","
       << "\"mode\":\"" << robotModeToString(current_mode) << "\","
       << "\"mode_id\":" << (int)current_mode << ","
       << "\"twist\":{\"vx\":" << current_twist.vx << ",\"vy\":" << current_twist.vy << ",\"w\":" << current_twist.omega << "},"
       << "\"wheels\":{\"fl\":" << current_wheels.front_left << ",\"fr\":" << current_wheels.front_right
       << ",\"rl\":" << current_wheels.rear_left << ",\"rr\":" << current_wheels.rear_right << "},"
       << "\"arm_pose\":{\"x\":" << current_pose.x << ",\"y\":" << current_pose.y << ",\"z\":" << current_pose.z
       << ",\"pitch\":" << current_pose.pitch << ",\"gripper\":" << current_pose.gripper << "},"
       << "\"arm_joints\":{\"j1\":" << current_joints.base_yaw << ",\"j2\":" << current_joints.shoulder_pitch
       << ",\"j3\":" << current_joints.elbow_pitch << ",\"j4\":" << current_joints.wrist_pitch
       << ",\"j5\":" << current_joints.wrist_roll << ",\"j6\":" << current_joints.gripper << "},"
       << "\"obstacle_dist_cm\":" << obstacle_distance_cm << ","
       << "\"e_stop\":" << (is_e_stopped ? "true" : "false") << ","
       << "\"braking\":" << (is_braking_for_obstacle ? "true" : "false") << ","
       << "\"pca9685\":" << (ServoController::getInstance().isPCA9685Active() ? "true" : "false")
       << "}";
    return ss.str();
}

} // namespace TamimysticOS
