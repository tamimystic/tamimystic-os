if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "port_esp32/os_scheduler_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core freertos)
else()
    add_library(os_scheduler STATIC "port_native/os_scheduler_native.cpp")
    target_include_directories(os_scheduler PUBLIC "include")
    target_link_libraries(os_scheduler PRIVATE os_hal os_core)
endif()
