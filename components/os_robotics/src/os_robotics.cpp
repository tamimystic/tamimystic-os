#include "os_robotics.h"
#include "os_hal_uart.h"
#include "os_hal_pwm.h"
#include "os_hal_gpio.h"
#include "os_cli.h"
#include <string>
#include <sstream>

namespace TamimysticOS {

// Assuming Motor Driver uses IN1, IN2, ENA for Left and IN3, IN4, ENB for Right.
#define MOTOR_LEFT_IN1 18
#define MOTOR_LEFT_IN2 19
#define MOTOR_LEFT_PWM 21
#define MOTOR_RIGHT_IN3 22
#define MOTOR_RIGHT_IN4 23
#define MOTOR_RIGHT_PWM 25

#define PWM_CHANNEL_LEFT 0
#define PWM_CHANNEL_RIGHT 1

MotorDriver& MotorDriver::getInstance() {
    static MotorDriver instance;
    return instance;
}

void MotorDriver::init() {
    hal_uart_print("[ROBOTICS] Initializing Motor Driver subsystem...\n");
    
    // Initialize GPIOs
    hal_gpio_set_direction(MOTOR_LEFT_IN1, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set_direction(MOTOR_LEFT_IN2, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set_direction(MOTOR_RIGHT_IN3, HAL_GPIO_MODE_OUTPUT);
    hal_gpio_set_direction(MOTOR_RIGHT_IN4, HAL_GPIO_MODE_OUTPUT);
    
    // Initialize PWM (10 kHz, 8-bit resolution)
    hal_pwm_init(MOTOR_LEFT_PWM, PWM_CHANNEL_LEFT, 10000, 8);
    hal_pwm_init(MOTOR_RIGHT_PWM, PWM_CHANNEL_RIGHT, 10000, 8);

    setSpeed(0, 0); // Ensure motors are stopped initially

    // Register CLI command
    CLI::getInstance().registerCommand("motor", "Set motor speeds (L R)", [](const std::vector<std::string>& args) {
        if (args.size() == 2) {
            int left = std::stoi(args[0]);
            int right = std::stoi(args[1]);
            MotorDriver::getInstance().setSpeed(left, right);
            hal_uart_print("Motors updated.\n");
        } else {
            hal_uart_print("Usage: motor <left_speed> <right_speed>\n");
        }
    });

    hal_uart_print("[ROBOTICS] Ready.\n");
}

void MotorDriver::setSpeed(int left_speed, int right_speed) {
    // Left motor direction
    if (left_speed >= 0) {
        hal_gpio_set_level(MOTOR_LEFT_IN1, 1);
        hal_gpio_set_level(MOTOR_LEFT_IN2, 0);
    } else {
        hal_gpio_set_level(MOTOR_LEFT_IN1, 0);
        hal_gpio_set_level(MOTOR_LEFT_IN2, 1);
        left_speed = -left_speed;
    }

    // Right motor direction
    if (right_speed >= 0) {
        hal_gpio_set_level(MOTOR_RIGHT_IN3, 1);
        hal_gpio_set_level(MOTOR_RIGHT_IN4, 0);
    } else {
        hal_gpio_set_level(MOTOR_RIGHT_IN3, 0);
        hal_gpio_set_level(MOTOR_RIGHT_IN4, 1);
        right_speed = -right_speed;
    }

    // Apply PWM
    hal_pwm_set_duty(PWM_CHANNEL_LEFT, (left_speed > 100) ? 100 : left_speed);
    hal_pwm_set_duty(PWM_CHANNEL_RIGHT, (right_speed > 100) ? 100 : right_speed);
}

} // namespace TamimysticOS
