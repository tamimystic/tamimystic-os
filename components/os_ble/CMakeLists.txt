if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_ble.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_robotics os_pnp bt freertos nvs_flash)
else()
    add_library(os_ble STATIC "src/os_ble.cpp")
    target_include_directories(os_ble PUBLIC "include")
    target_link_libraries(os_ble PRIVATE os_hal os_core os_scheduler os_robotics os_pnp)
endif()
