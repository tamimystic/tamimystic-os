#include "os_hal_gpio.h"
#include "driver/gpio.h"

extern "C" {

void hal_gpio_set_direction(int pin, hal_gpio_mode_t mode) {
    gpio_set_direction((gpio_num_t)pin, (mode == HAL_GPIO_MODE_OUTPUT) ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
}

void hal_gpio_set_level(int pin, int level) {
    gpio_set_level((gpio_num_t)pin, level);
}

int hal_gpio_get_level(int pin) {
    return gpio_get_level((gpio_num_t)pin);
}

bool hal_gpio_is_valid(int pin) {
    return GPIO_IS_VALID_GPIO(pin);
}

bool hal_gpio_is_safe_user_pin(int pin) {
    if (!hal_gpio_is_valid(pin)) return false;
    // On ESP32-S3 N16R8, GPIO 33-37 are dedicated for Octal PSRAM/Flash!
    if (pin >= 33 && pin <= 37) return false;
    // USB pins GPIO 19, 20 if used for USB
    if (pin == 19 || pin == 20) return false;
    // Strapping pins (GPIO 0, 45, 46) - permit with caution, but 45/46 should generally be avoided if possible
    if (pin == 45 || pin == 46) return false;
    return true;
}

}
