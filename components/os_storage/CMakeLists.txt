if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "port_esp32/os_storage_esp32.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core spiffs)
else()
    add_library(os_storage STATIC "port_native/os_storage_native.cpp")
    target_include_directories(os_storage PUBLIC "include")
    target_link_libraries(os_storage PRIVATE os_hal os_core)
endif()
