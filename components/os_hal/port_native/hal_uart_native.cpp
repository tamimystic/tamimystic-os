#include "os_hal_uart.h"
#include <iostream>

extern "C" {

void hal_uart_init(void) {
    // For Native POSIX, standard output is automatically initialized by the OS.
    std::cout << "[HAL_NATIVE] UART Mock Initialized." << std::endl;
}

void hal_uart_print(const char* str) {
    std::cout << str;
}

void hal_uart_read_line(char* buffer, int max_len) {
    std::string line;
    if (std::getline(std::cin, line)) {
        int copy_len = std::min((int)line.length(), max_len - 1);
        for (int i = 0; i < copy_len; ++i) {
            buffer[i] = line[i];
        }
        buffer[copy_len] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

} // extern "C"
