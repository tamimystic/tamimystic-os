#include "os_hal_i2c.h"
#include "os_hal_uart.h"
#include <string>

extern "C" {

void hal_i2c_master_init(int sda_pin, int scl_pin, int clk_speed_hz) {
    hal_uart_print(("[HAL:I2C] Master Init SDA=" + std::to_string(sda_pin) + 
                    " SCL=" + std::to_string(scl_pin) + " Freq=" + std::to_string(clk_speed_hz) + "Hz\n").c_str());
}

bool hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    hal_uart_print(("[HAL:I2C] Read from Dev " + std::to_string(dev_addr) + " Reg " + std::to_string(reg_addr) + "\n").c_str());
    return true;
}

bool hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    hal_uart_print(("[HAL:I2C] Write to Dev " + std::to_string(dev_addr) + " Reg " + std::to_string(reg_addr) + "\n").c_str());
    return true;
}

}
