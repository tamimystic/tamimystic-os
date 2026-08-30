if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_python_runner.cpp"
                                "src/os_wasm_runner.cpp"
                                "src/os_apps.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_cli os_storage os_robotics os_pnp os_ai)
else()
    add_library(os_apps STATIC "src/os_python_runner.cpp"
                               "src/os_wasm_runner.cpp"
                               "src/os_apps.cpp")
    target_include_directories(os_apps PUBLIC "include")
    target_link_libraries(os_apps PRIVATE os_hal os_core os_scheduler os_cli os_storage os_robotics os_pnp os_ai)
endif()
