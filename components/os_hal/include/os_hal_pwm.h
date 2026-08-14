#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void hal_pwm_init(int pin, int channel, int frequency, int resolution_bits);
void hal_pwm_set_duty(int channel, int duty_percent);

#ifdef __cplusplus
}
#endif
