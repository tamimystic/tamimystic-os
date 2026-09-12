if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_slam.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_robotics os_pnp driver)
else()
    add_library(os_slam STATIC "src/os_slam.cpp")
    target_include_directories(os_slam PUBLIC "include")
    target_link_libraries(os_slam PRIVATE os_hal os_core os_scheduler os_robotics os_pnp)
endif()
