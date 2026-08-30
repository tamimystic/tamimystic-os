#include "os_hal_gpio.h"
#include "os_hal_uart.h"
#include <string>

extern "C" {

void hal_gpio_set_direction(int pin, hal_gpio_mode_t mode) {
    std::string mode_str = (mode == HAL_GPIO_MODE_OUTPUT) ? "OUTPUT" : "INPUT";
    hal_uart_print(("[HAL:GPIO] Pin " + std::to_string(pin) + " mode set to " + mode_str + "\n").c_str());
}

static int pin_levels[64] = {-1};

void hal_gpio_set_level(int pin, int level) {
    if (pin >= 0 && pin < 64) {
        if (pin_levels[pin] == level) return; // Skip duplicate output
        pin_levels[pin] = level;
    }
    hal_uart_print(("[HAL:GPIO] Pin " + std::to_string(pin) + " level set to " + std::to_string(level) + "\n").c_str());
}

int hal_gpio_get_level(int pin) {
    if (pin >= 0 && pin < 64 && pin_levels[pin] >= 0) {
        return pin_levels[pin];
    }
    return 0;
}

bool hal_gpio_is_valid(int pin) {
    return (pin >= 0 && pin <= 48);
}

bool hal_gpio_is_safe_user_pin(int pin) {
    if (!hal_gpio_is_valid(pin)) return false;
    // Disallow PSRAM/Flash dedicated pins and USB pins for accurate ESP32-S3 simulation
    if (pin >= 33 && pin <= 37) return false;
    if (pin == 19 || pin == 20) return false;
    if (pin == 45 || pin == 46) return false;
    return true;
}

}
