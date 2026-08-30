#pragma once

#include "os_robotics_types.h"
#include <cstdint>
#include <vector>

namespace TamimysticOS {

class ServoController {
public:
    static ServoController& getInstance();

    void init();

    // Set individual servo angle (0 - 180 degrees)
    void setAngle(uint8_t channel, float angle_deg);

    // Apply entire set of robotic arm joint angles
    void applyArmJoints(const ArmJoints& joints);

    // Check if PCA9685 16-channel I2C controller is active
    bool isPCA9685Active() const;

    // Get current commanded joint angles
    ArmJoints getCurrentJoints() const;

private:
    ServoController() = default;
    ~ServoController() = default;

    void initPCA9685();
    void writePCA9685Channel(uint8_t channel, uint16_t on_step, uint16_t off_step);

    bool pca9685_present = false;
    uint8_t pca9685_addr = 0x40;
    ArmJoints current_joints;
};

} // namespace TamimysticOS
