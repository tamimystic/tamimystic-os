#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace TamimysticOS {

enum class DeviceCategory {
    UNKNOWN = 0,
    IMU_MOTION,         // 6-DOF / 9-DOF IMU (e.g. MPU6050, MPU9250, BNO085)
    SERVO_DRIVER,       // 16-Ch PWM Servo controller (e.g. PCA9685)
    DISPLAY_OLED,       // OLED Displays (SSD1306, SH1106)
    DISTANCE_TOF,       // Laser / ToF Distance (VL53L0X, VL53L1X)
    ENVIRONMENTAL,      // Temp/Humidity/Pressure (BME280, BMP280, AHT20)
    POWER_MONITOR,      // Voltage/Current monitor (INA219, INA226)
    IO_EXPANDER,        // GPIO expanders (PCF8574, MCP23017)
    PRECISION_ADC,      // 16-Bit ADC (ADS1115)
    LIGHT_SENSOR        // Ambient Light (BH1750)
};

inline const char* deviceCategoryToString(DeviceCategory cat) {
    switch (cat) {
        case DeviceCategory::IMU_MOTION:     return "IMU & Motion";
        case DeviceCategory::SERVO_DRIVER:   return "Servo Controller";
        case DeviceCategory::DISPLAY_OLED:   return "OLED Display";
        case DeviceCategory::DISTANCE_TOF:   return "ToF Laser Distance";
        case DeviceCategory::ENVIRONMENTAL:  return "Environmental Sensor";
        case DeviceCategory::POWER_MONITOR:  return "Power Monitor";
        case DeviceCategory::IO_EXPANDER:    return "I/O Expander";
        case DeviceCategory::PRECISION_ADC:  return "Precision ADC";
        case DeviceCategory::LIGHT_SENSOR:   return "Ambient Light";
        default:                             return "Unknown Peripheral";
    }
}

inline const char* deviceCategoryToIcon(DeviceCategory cat) {
    switch (cat) {
        case DeviceCategory::IMU_MOTION:     return "[IMU]";
        case DeviceCategory::SERVO_DRIVER:   return "[SERVO]";
        case DeviceCategory::DISPLAY_OLED:   return "[OLED]";
        case DeviceCategory::DISTANCE_TOF:   return "[TOF]";
        case DeviceCategory::ENVIRONMENTAL:  return "[ENV]";
        case DeviceCategory::POWER_MONITOR:  return "[PWR]";
        case DeviceCategory::IO_EXPANDER:    return "[IO]";
        case DeviceCategory::PRECISION_ADC:  return "[ADC]";
        case DeviceCategory::LIGHT_SENSOR:   return "[LIGHT]";
        default:                             return "[DEV]";
    }
}

struct DiscoveredDevice {
    uint8_t address = 0;
    std::string name;
    std::string model;
    DeviceCategory category = DeviceCategory::UNKNOWN;
    std::string driver_name;
    bool active = false;
    std::string live_reading;
};

enum class PinFunction {
    UNASSIGNED = 0,
    I2C_SDA,
    I2C_SCL,
    MOTOR_L_IN1,
    MOTOR_L_IN2,
    MOTOR_L_PWM,
    MOTOR_R_IN3,
    MOTOR_R_IN4,
    MOTOR_R_PWM,
    MOTOR_FL_PWM,
    MOTOR_FL_IN1,
    MOTOR_FL_IN2,
    MOTOR_FR_PWM,
    MOTOR_FR_IN1,
    MOTOR_FR_IN2,
    MOTOR_RL_PWM,
    MOTOR_RL_IN1,
    MOTOR_RL_IN2,
    MOTOR_RR_PWM,
    MOTOR_RR_IN1,
    MOTOR_RR_IN2,
    SERVO_PWM,
    ENCODER_L_A,
    ENCODER_L_B,
    ENCODER_R_A,
    ENCODER_R_B,
    ULTRASONIC_TRIG,
    ULTRASONIC_ECHO,
    STATUS_LED,
    BUZZER,
    I2S_BCLK,
    I2S_WS,
    I2S_DIN,
    I2S_DOUT
};

inline const char* pinFunctionToString(PinFunction func) {
    switch (func) {
        case PinFunction::I2C_SDA:          return "I2C SDA";
        case PinFunction::I2C_SCL:          return "I2C SCL";
        case PinFunction::MOTOR_L_IN1:      return "Motor Left IN1";
        case PinFunction::MOTOR_L_IN2:      return "Motor Left IN2";
        case PinFunction::MOTOR_L_PWM:      return "Motor Left PWM";
        case PinFunction::MOTOR_R_IN3:      return "Motor Right IN3";
        case PinFunction::MOTOR_R_IN4:      return "Motor Right IN4";
        case PinFunction::MOTOR_R_PWM:      return "Motor Right PWM";
        case PinFunction::MOTOR_FL_PWM:     return "Motor Front-Left PWM";
        case PinFunction::MOTOR_FL_IN1:     return "Motor Front-Left IN1";
        case PinFunction::MOTOR_FL_IN2:     return "Motor Front-Left IN2";
        case PinFunction::MOTOR_FR_PWM:     return "Motor Front-Right PWM";
        case PinFunction::MOTOR_FR_IN1:     return "Motor Front-Right IN1";
        case PinFunction::MOTOR_FR_IN2:     return "Motor Front-Right IN2";
        case PinFunction::MOTOR_RL_PWM:     return "Motor Rear-Left PWM";
        case PinFunction::MOTOR_RL_IN1:     return "Motor Rear-Left IN1";
        case PinFunction::MOTOR_RL_IN2:     return "Motor Rear-Left IN2";
        case PinFunction::MOTOR_RR_PWM:     return "Motor Rear-Right PWM";
        case PinFunction::MOTOR_RR_IN1:     return "Motor Rear-Right IN1";
        case PinFunction::MOTOR_RR_IN2:     return "Motor Rear-Right IN2";
        case PinFunction::SERVO_PWM:        return "Servo PWM";
        case PinFunction::ENCODER_L_A:      return "Encoder Left A";
        case PinFunction::ENCODER_L_B:      return "Encoder Left B";
        case PinFunction::ENCODER_R_A:      return "Encoder Right A";
        case PinFunction::ENCODER_R_B:      return "Encoder Right B";
        case PinFunction::ULTRASONIC_TRIG:  return "Ultrasonic Trigger";
        case PinFunction::ULTRASONIC_ECHO:  return "Ultrasonic Echo";
        case PinFunction::STATUS_LED:       return "Status LED";
        case PinFunction::BUZZER:           return "Buzzer";
        case PinFunction::I2S_BCLK:         return "I2S Bit Clock (BCLK)";
        case PinFunction::I2S_WS:           return "I2S Word Select (WS)";
        case PinFunction::I2S_DIN:          return "I2S Mic Data (DIN)";
        case PinFunction::I2S_DOUT:         return "I2S Speaker Data (DOUT)";
        default:                            return "Unassigned";
    }
}

} // namespace TamimysticOS
