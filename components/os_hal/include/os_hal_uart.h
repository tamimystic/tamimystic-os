#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the hardware UART for system logging/CLI.
 */
void hal_uart_init(void);

/**
 * @brief Print a string out via the hardware UART.
 * 
 * @param str Null-terminated string to print.
 */
void hal_uart_print(const char* str);

/**
 * @brief Read a line of text from the hardware UART. Blocks until newline is received.
 * 
 * @param buffer Buffer to store the read string.
 * @param max_len Maximum number of characters to read.
 */
void hal_uart_read_line(char* buffer, int max_len);

#ifdef __cplusplus
}
#endif
