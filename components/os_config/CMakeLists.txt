if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "port_esp32/os_config_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core nvs_flash)
else()
    add_library(os_config STATIC "port_native/os_config_native.cpp")
    target_include_directories(os_config PUBLIC "include")
    target_link_libraries(os_config PRIVATE os_hal os_core)
endif()
