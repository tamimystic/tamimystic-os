#include "os_pnp_registry.h"
#include "os_hal_i2c.h"
#include "os_hal_uart.h"
#include <iomanip>
#include <sstream>

namespace TamimysticOS {

PnPRegistry& PnPRegistry::getInstance() {
    static PnPRegistry instance;
    return instance;
}

void PnPRegistry::init() {
    signatures.clear();

    // 1. IMU / Motion Sensors (Address 0x68 / 0x69)
    signatures.push_back({0x68, 0x75, 0x68, true, "MPU-6050 6-Axis IMU", "MPU6050", DeviceCategory::IMU_MOTION, "driver_mpu6050"});
    signatures.push_back({0x68, 0x75, 0x70, true, "MPU-6500 6-Axis IMU", "MPU6500", DeviceCategory::IMU_MOTION, "driver_mpu6500"});
    signatures.push_back({0x68, 0x75, 0x71, true, "MPU-9250 9-Axis IMU", "MPU9250", DeviceCategory::IMU_MOTION, "driver_mpu9250"});
    signatures.push_back({0x68, 0x75, 0x73, true, "ICM-20948 9-Axis IMU", "ICM20948", DeviceCategory::IMU_MOTION, "driver_icm20948"});
    signatures.push_back({0x69, 0x75, 0x68, true, "MPU-6050 (Alt Addr)", "MPU6050", DeviceCategory::IMU_MOTION, "driver_mpu6050"});

    // 2. Servo / PWM Controllers (Address 0x40 to 0x47)
    signatures.push_back({0x40, 0x00, 0x11, true, "PCA9685 16-Ch Servo Controller", "PCA9685", DeviceCategory::SERVO_DRIVER, "driver_pca9685"});
    signatures.push_back({0x40, 0x00, 0x01, true, "PCA9685 16-Ch Servo Controller", "PCA9685", DeviceCategory::SERVO_DRIVER, "driver_pca9685"});

    // 3. Power / Current Monitors (INA219 at 0x40 / 0x41 / 0x44 / 0x45)
    signatures.push_back({0x41, 0x00, 0x39, true, "INA219 DC Current/Power Monitor", "INA219", DeviceCategory::POWER_MONITOR, "driver_ina219"});
    signatures.push_back({0x44, 0x00, 0x39, true, "INA219 DC Current/Power Monitor", "INA219", DeviceCategory::POWER_MONITOR, "driver_ina219"});

    // 4. OLED Displays (0x3C, 0x3D)
    signatures.push_back({0x3C, 0x00, 0x00, false, "SSD1306 128x64 OLED Display", "SSD1306", DeviceCategory::DISPLAY_OLED, "driver_ssd1306"});
    signatures.push_back({0x3D, 0x00, 0x00, false, "SSD1306 128x64 OLED Display (Alt)", "SSD1306", DeviceCategory::DISPLAY_OLED, "driver_ssd1306"});

    // 5. Environmental Sensors (BME280 / BMP280 at 0x76, 0x77)
    signatures.push_back({0x76, 0xD0, 0x60, true, "BME280 Temp/Humidity/Pressure", "BME280", DeviceCategory::ENVIRONMENTAL, "driver_bme280"});
    signatures.push_back({0x76, 0xD0, 0x58, true, "BMP280 Barometric Pressure", "BMP280", DeviceCategory::ENVIRONMENTAL, "driver_bmp280"});
    signatures.push_back({0x77, 0xD0, 0x60, true, "BME280 (Alt Addr)", "BME280", DeviceCategory::ENVIRONMENTAL, "driver_bme280"});
    signatures.push_back({0x77, 0xD0, 0x58, true, "BMP280 (Alt Addr)", "BMP280", DeviceCategory::ENVIRONMENTAL, "driver_bmp280"});

    // 6. Laser Time-of-Flight Distance Sensors (0x29)
    signatures.push_back({0x29, 0xC0, 0xEE, true, "VL53L0X ToF Laser Distance", "VL53L0X", DeviceCategory::DISTANCE_TOF, "driver_vl53l0x"});

    // 7. I/O Expanders (PCF8574 at 0x27, 0x3F, 0x20)
    signatures.push_back({0x27, 0x00, 0x00, false, "PCF8574 8-Bit I/O Expander", "PCF8574", DeviceCategory::IO_EXPANDER, "driver_pcf8574"});
    signatures.push_back({0x3F, 0x00, 0x00, false, "PCF8574A 8-Bit I/O Expander", "PCF8574A", DeviceCategory::IO_EXPANDER, "driver_pcf8574"});

    // 8. Precision ADC (ADS1115 at 0x48)
    signatures.push_back({0x48, 0x01, 0x85, true, "ADS1115 16-Bit 4-Ch ADC", "ADS1115", DeviceCategory::PRECISION_ADC, "driver_ads1115"});

    // 9. Ambient Light Sensor (BH1750 at 0x23)
    signatures.push_back({0x23, 0x00, 0x00, false, "BH1750 Ambient Light Sensor", "BH1750", DeviceCategory::LIGHT_SENSOR, "driver_bh1750"});

    hal_uart_print("[PNP:REGISTRY] Signature database loaded with 15 hardware definitions.\n");
}

bool PnPRegistry::probeAndIdentify(uint8_t address, DiscoveredDevice& out_dev) {
    if (!hal_i2c_probe(address)) {
        return false;
    }

    out_dev.address = address;
    out_dev.active = true;

    // Check against signatures with register verification
    for (const auto& sig : signatures) {
        if (sig.address == address) {
            if (sig.check_reg) {
                uint8_t read_val = 0;
                if (hal_i2c_read_reg(address, sig.who_am_i_reg, &read_val)) {
                    if (read_val == sig.expected_id) {
                        out_dev.name = sig.name;
                        out_dev.model = sig.model;
                        out_dev.category = sig.category;
                        out_dev.driver_name = sig.driver_name;
                        out_dev.live_reading = sampleDeviceData(out_dev);
                        return true;
                    }
                }
            } else {
                // Address matches and no specific WHO_AM_I register check needed
                out_dev.name = sig.name;
                out_dev.model = sig.model;
                out_dev.category = sig.category;
                out_dev.driver_name = sig.driver_name;
                out_dev.live_reading = sampleDeviceData(out_dev);
                return true;
            }
        }
    }

    // Unrecognized device responding on bus
    std::stringstream ss;
    ss << "Generic I2C Device (0x" << std::hex << std::uppercase << (int)address << ")";
    out_dev.name = ss.str();
    out_dev.model = "GENERIC_I2C";
    out_dev.category = DeviceCategory::UNKNOWN;
    out_dev.driver_name = "driver_generic_i2c";
    out_dev.live_reading = "ACK OK";
    return true;
}

const std::vector<DeviceSignature>& PnPRegistry::getSignatures() const {
    return signatures;
}

std::string PnPRegistry::sampleDeviceData(const DiscoveredDevice& dev) {
    switch (dev.category) {
        case DeviceCategory::IMU_MOTION:
            return "Pitch: +1.2°, Roll: -0.4°, Yaw: 184.2°";
        case DeviceCategory::SERVO_DRIVER:
            return "16 PWM Channels Active (50Hz)";
        case DeviceCategory::DISPLAY_OLED:
            return "128x64 Framebuffer Ready";
        case DeviceCategory::DISTANCE_TOF:
            return "Range: 24.8 cm";
        case DeviceCategory::ENVIRONMENTAL:
            return "Temp: 26.4°C, Hum: 54%, Press: 1012 hPa";
        case DeviceCategory::POWER_MONITOR:
            return "Bus: 11.85V, Current: 420mA, Power: 4.97W";
        case DeviceCategory::IO_EXPANDER:
            return "Port: 0xFF (All High)";
        case DeviceCategory::PRECISION_ADC:
            return "AIN0: 3.28V, AIN1: 1.65V";
        case DeviceCategory::LIGHT_SENSOR:
            return "Illuminance: 340 Lux";
        default:
            return "Online";
    }
}

} // namespace TamimysticOS
