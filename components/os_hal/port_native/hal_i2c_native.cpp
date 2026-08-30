#include "os_hal_i2c.h"
#include "os_hal_uart.h"
#include <string>
#include <unordered_map>
#include <vector>

// Simulated virtual I2C peripherals on Native PC
static const std::unordered_map<uint8_t, std::unordered_map<uint8_t, uint8_t>> simulated_devices = {
    {0x68, {{0x75, 0x68}}}, // MPU6050: WHO_AM_I reg 0x75 = 0x68
    {0x40, {{0x00, 0x11}}}, // PCA9685: MODE1 reg 0x00 = 0x11 (Sleep/Restart bits)
    {0x3C, {{0x00, 0x00}}}, // SSD1306 OLED Display
    {0x76, {{0xD0, 0x60}}}, // BME280: Chip ID reg 0xD0 = 0x60
    {0x29, {{0xC0, 0xEE}}}  // VL53L0X: Model ID reg 0xC0 = 0xEE
};

extern "C" {

void hal_i2c_master_init(int sda_pin, int scl_pin, int clk_speed_hz) {
    hal_uart_print(("[HAL:I2C] Master Init SDA=GPIO" + std::to_string(sda_pin) + 
                    " SCL=GPIO" + std::to_string(scl_pin) + " Freq=" + std::to_string(clk_speed_hz) + "Hz\n").c_str());
}

void hal_i2c_deinit() {
    hal_uart_print("[HAL:I2C] Master Deinitialized.\n");
}

bool hal_i2c_probe(uint8_t dev_addr) {
    // Check if simulated device exists at this address
    return simulated_devices.find(dev_addr) != simulated_devices.end();
}

bool hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    auto dev_it = simulated_devices.find(dev_addr);
    if (dev_it == simulated_devices.end()) {
        return false;
    }
    if (data && len > 0) {
        auto reg_it = dev_it->second.find(reg_addr);
        if (reg_it != dev_it->second.end()) {
            data[0] = reg_it->second;
        } else {
            data[0] = 0x00;
        }
        for (size_t i = 1; i < len; ++i) {
            data[i] = 0x00;
        }
    }
    return true;
}

bool hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    auto dev_it = simulated_devices.find(dev_addr);
    if (dev_it == simulated_devices.end()) {
        return false;
    }
    return true;
}

bool hal_i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *val) {
    return hal_i2c_read(dev_addr, reg_addr, val, 1);
}

bool hal_i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t val) {
    return hal_i2c_write(dev_addr, reg_addr, &val, 1);
}

}
