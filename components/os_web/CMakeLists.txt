if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "port_esp32/os_web_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_network os_robotics os_ai os_storage esp_http_server)
else()
    add_library(os_web STATIC "port_native/os_web_native.cpp")
    target_include_directories(os_web PUBLIC "include")
    target_link_libraries(os_web PRIVATE os_hal os_core os_network os_robotics os_ai os_storage ws2_32)
endif()
