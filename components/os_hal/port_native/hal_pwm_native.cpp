#include "os_hal_pwm.h"
#include "os_hal_uart.h"
#include <string>

extern "C" {

void hal_pwm_init(int pin, int channel, int frequency, int resolution_bits) {
    hal_uart_print(("[HAL:PWM] Init Pin " + std::to_string(pin) + " Ch " + std::to_string(channel) + 
                    " Freq " + std::to_string(frequency) + "Hz Res " + std::to_string(resolution_bits) + "bits\n").c_str());
}

void hal_pwm_set_duty(int channel, int duty_percent) {
    hal_uart_print(("[HAL:PWM] Channel " + std::to_string(channel) + " duty set to " + std::to_string(duty_percent) + "%\n").c_str());
}

}
