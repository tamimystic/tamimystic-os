#pragma once
#include <stdint.h>

namespace TamimysticOS {

class MotorDriver {
public:
    static MotorDriver& getInstance();

    // Initialize motor driver pins and PWM channels
    void init();

    // Set motor speed. Values range from -100 (full reverse) to 100 (full forward)
    void setSpeed(int left_motor_speed, int right_motor_speed);

private:
    MotorDriver() = default;
    ~MotorDriver() = default;
};

} // namespace TamimysticOS
