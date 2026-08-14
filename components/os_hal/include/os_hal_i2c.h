#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_i2c_master_init(int sda_pin, int scl_pin, int clk_speed_hz);
bool hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len);
bool hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif
