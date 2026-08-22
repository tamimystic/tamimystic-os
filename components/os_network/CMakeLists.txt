if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "port_esp32/os_network_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_cli os_config esp_wifi nvs_flash lwip)
else()
    add_library(os_network STATIC "port_native/os_network_native.cpp")
    target_include_directories(os_network PUBLIC "include")
    target_link_libraries(os_network PRIVATE os_hal os_core os_cli os_config)
endif()
