if(DEFINED ENV{IDF_PATH})
    # ESP-IDF specific build
    idf_component_register(SRCS "port_esp32/hal_uart_esp32.cpp"
                                "port_esp32/hal_gpio_esp32.cpp"
                                "port_esp32/hal_pwm_esp32.cpp"
                                "port_esp32/hal_i2c_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES driver)
else()
    # Native POSIX build
    add_library(os_hal STATIC "port_native/hal_uart_native.cpp"
                              "port_native/hal_gpio_native.cpp"
                              "port_native/hal_pwm_native.cpp"
                              "port_native/hal_i2c_native.cpp")
    target_include_directories(os_hal PUBLIC "include")
endif()
