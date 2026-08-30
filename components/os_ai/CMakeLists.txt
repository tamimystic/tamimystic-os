if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_camera.cpp"
                                "src/os_ai.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_pnp os_robotics)
else()
    add_library(os_ai STATIC "src/os_camera.cpp"
                             "src/os_ai.cpp")
    target_include_directories(os_ai PUBLIC "include")
    target_link_libraries(os_ai PRIVATE os_hal os_core os_scheduler os_pnp os_robotics)
endif()
