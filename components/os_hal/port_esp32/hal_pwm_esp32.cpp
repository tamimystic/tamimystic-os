#include "os_hal_pwm.h"
#include "driver/ledc.h"

extern "C" {

void hal_pwm_init(int pin, int channel, int frequency, int resolution) {
    ledc_timer_config_t timer_conf = {};
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.duty_resolution = (ledc_timer_bit_t)resolution;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = (uint32_t)frequency;
    timer_conf.clk_cfg = LEDC_AUTO_CLK;
    
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t ch_conf = {};
    ch_conf.gpio_num = pin;
    ch_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_conf.channel = (ledc_channel_t)channel;
    ch_conf.intr_type = LEDC_INTR_DISABLE;
    ch_conf.timer_sel = LEDC_TIMER_0;
    ch_conf.duty = 0;
    ch_conf.hpoint = 0;

    ledc_channel_config(&ch_conf);
}

void hal_pwm_set_duty(int channel, int duty_percent) {
    // Assuming 8-bit resolution (0-255) maps to 0-100%
    // Formula: (duty_percent * 255) / 100
    uint32_t duty_val = (duty_percent * 255) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel, duty_val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)channel);
}

}
