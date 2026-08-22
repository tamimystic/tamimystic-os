if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "main.cpp"
                           INCLUDE_DIRS "."
                           REQUIRES os_hal os_core os_scheduler os_cli os_config os_network os_storage os_web os_apps os_robotics os_ai)
else()
    add_executable(tamimystic_os_sim main.cpp)
    target_link_libraries(tamimystic_os_sim PRIVATE os_hal os_core os_scheduler os_cli os_config os_network os_storage os_web os_apps os_robotics os_ai)
endif()
