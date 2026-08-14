#include "os_hal_gpio.h"
#include "os_hal_uart.h"
#include <string>

extern "C" {

void hal_gpio_set_direction(int pin, hal_gpio_mode_t mode) {
    std::string mode_str = (mode == HAL_GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT";
    hal_uart_print(("[HAL:GPIO] Pin " + std::to_string(pin) + " mode set to " + mode_str + "\n").c_str());
}

void hal_gpio_set_level(int pin, int level) {
    hal_uart_print(("[HAL:GPIO] Pin " + std::to_string(pin) + " level set to " + std::to_string(level) + "\n").c_str());
}

int hal_gpio_get_level(int pin) {
    return 0; // Mock always returns 0
}

}
