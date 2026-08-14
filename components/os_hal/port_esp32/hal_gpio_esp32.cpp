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

}
