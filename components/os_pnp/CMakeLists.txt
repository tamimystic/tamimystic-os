if(DEFINED ENV{IDF_PATH})
    # ESP-IDF Target Build
    idf_component_register(SRCS "src/os_pnp_registry.cpp"
                                "src/os_pin_matrix.cpp"
                                "src/os_pnp_manager.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_config)
else()
    # Native POSIX / Windows Build
    add_library(os_pnp STATIC "src/os_pnp_registry.cpp"
                              "src/os_pin_matrix.cpp"
                              "src/os_pnp_manager.cpp")
    target_include_directories(os_pnp PUBLIC "include")
    target_link_libraries(os_pnp PRIVATE os_hal os_core os_config)
endif()
