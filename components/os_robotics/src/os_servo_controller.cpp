#include "os_servo_controller.h"
#include "os_hal_i2c.h"
#include "os_hal_pwm.h"
#include "os_hal_uart.h"
#include "os_pin_matrix.h"
#include <algorithm>
#include <cmath>

#define PCA9685_MODE1       0x00
#define PCA9685_PRESCALE    0xFE
#define PCA9685_LED0_ON_L   0x06

namespace TamimysticOS {

ServoController& ServoController::getInstance() {
    static ServoController instance;
    return instance;
}

void ServoController::init() {
    hal_uart_print("[SERVO] Initializing Multi-Channel Servo Subsystem...\n");

    // Check if PCA9685 16-channel controller is detected on I2C bus (Address 0x40)
    if (hal_i2c_probe(pca9685_addr)) {
        pca9685_present = true;
        initPCA9685();
        hal_uart_print("[SERVO] Auto-linked to PCA9685 16-Channel I2C Servo Expander at 0x40.\n");
    } else {
        pca9685_present = false;
        hal_uart_print("[SERVO] PCA9685 not detected. Falling back to onboard ESP32 PWM servo pins.\n");
        int servo_pin = PinMatrixManager::getInstance().getPin(PinFunction::SERVO_PWM);
        if (servo_pin >= 0) {
            hal_pwm_init(servo_pin, 2, 50, 16); // 50 Hz, 16-bit PWM for standard RC servo
        }
    }

    // Initialize to default center arm pose
    applyArmJoints(current_joints);
    hal_uart_print("[SERVO] Ready.\n");
}

void ServoController::initPCA9685() {
    // 1. Set Sleep mode to change prescaler (50Hz = ~20ms period)
    hal_i2c_write_reg(pca9685_addr, PCA9685_MODE1, 0x10); // SLEEP
    // Oscillator is ~25MHz: Prescale = round(25000000 / (4096 * 50)) - 1 = 121 (0x79)
    hal_i2c_write_reg(pca9685_addr, PCA9685_PRESCALE, 0x79);
    // 2. Clear Sleep mode and auto-increment
    hal_i2c_write_reg(pca9685_addr, PCA9685_MODE1, 0xA1); // AUTO_INCREMENT | ALLCALL
}

void ServoController::writePCA9685Channel(uint8_t channel, uint16_t on_step, uint16_t off_step) {
    if (!pca9685_present || channel > 15) return;
    uint8_t reg = PCA9685_LED0_ON_L + 4 * channel;
    uint8_t buf[4] = {
        (uint8_t)(on_step & 0xFF),
        (uint8_t)((on_step >> 8) & 0xFF),
        (uint8_t)(off_step & 0xFF),
        (uint8_t)((off_step >> 8) & 0xFF)
    };
    hal_i2c_write(pca9685_addr, reg, buf, 4);
}

void ServoController::setAngle(uint8_t channel, float angle_deg) {
    angle_deg = std::clamp(angle_deg, 0.0f, 180.0f);

    // Standard 50Hz Servo timing: 0 deg = 500us, 180 deg = 2500us (out of 20000us period)
    // 4096 steps per 20ms -> 1 step = ~4.88us
    // 500us = ~102 steps, 2500us = ~512 steps
    uint16_t off_step = (uint16_t)(102.0f + (angle_deg / 180.0f) * (512.0f - 102.0f));

    if (pca9685_present) {
        writePCA9685Channel(channel, 0, off_step);
    } else {
        // Direct PWM fallback (Duty in %)
        float duty_pct = (0.5f + (angle_deg / 180.0f) * 2.0f) / 20.0f * 100.0f;
        hal_pwm_set_duty(2, (int)duty_pct);
    }
}

void ServoController::applyArmJoints(const ArmJoints& joints) {
    current_joints = joints;
    setAngle(0, current_joints.base_yaw);
    setAngle(1, current_joints.shoulder_pitch);
    setAngle(2, current_joints.elbow_pitch);
    setAngle(3, current_joints.wrist_pitch);
    setAngle(4, current_joints.wrist_roll);
    // Gripper mapped from 0-100% to 0-90 degrees
    setAngle(5, (current_joints.gripper / 100.0f) * 90.0f);
}

bool ServoController::isPCA9685Active() const {
    return pca9685_present;
}

ArmJoints ServoController::getCurrentJoints() const {
    return current_joints;
}

} // namespace TamimysticOS
