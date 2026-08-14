#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HAL_GPIO_MODE_INPUT,
    HAL_GPIO_MODE_OUTPUT
} hal_gpio_mode_t;

void hal_gpio_set_direction(int pin, hal_gpio_mode_t mode);
void hal_gpio_set_level(int pin, int level);
int hal_gpio_get_level(int pin);

#ifdef __cplusplus
}
#endif
