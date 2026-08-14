#include "os_hal_uart.h"
#include "esp_log.h"
#include <stdio.h>

static const char* TAG = "HAL_UART";

extern "C" {

void hal_uart_init(void) {
    // In ESP-IDF, the default UART0 is generally initialized by the bootloader
    // and standard I/O (printf) is already routed there.
    // Additional custom UART configuration can be done here using the driver/uart.h API.
    ESP_LOGI(TAG, "UART Initialized.");
}

void hal_uart_print(const char* str) {
    printf("%s", str);
    fflush(stdout);
}

void hal_uart_read_line(char* buffer, int max_len) {
    if (fgets(buffer, max_len, stdin) != NULL) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        if (len > 1 && buffer[len-2] == '\r') {
            buffer[len-2] = '\0';
        }
    } else {
        buffer[0] = '\0';
    }
}

} // extern "C"
