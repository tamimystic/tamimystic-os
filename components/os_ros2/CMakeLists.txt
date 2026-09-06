if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_ros2.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_config os_scheduler os_robotics os_pnp os_ai)
else()
    add_library(os_ros2 STATIC "src/os_ros2.cpp")
    target_include_directories(os_ros2 PUBLIC "include")
    target_link_libraries(os_ros2 PRIVATE os_hal os_core os_config os_scheduler os_robotics os_pnp os_ai)
endif()
