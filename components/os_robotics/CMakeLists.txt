if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_kinematics.cpp"
                                "src/os_servo_controller.cpp"
                                "src/os_robotics.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_cli os_pnp os_scheduler)
else()
    add_library(os_robotics STATIC "src/os_kinematics.cpp"
                                  "src/os_servo_controller.cpp"
                                  "src/os_robotics.cpp")
    target_include_directories(os_robotics PUBLIC "include")
    target_link_libraries(os_robotics PRIVATE os_hal os_core os_cli os_pnp os_scheduler)
endif()
