#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_i2c_master_init(int sda_pin, int scl_pin, int clk_speed_hz);
void hal_i2c_deinit();
bool hal_i2c_probe(uint8_t dev_addr);
bool hal_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len);
bool hal_i2c_write(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len);
bool hal_i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *val);
bool hal_i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t val);

#ifdef __cplusplus
}
#endif
