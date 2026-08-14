if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_robotics.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_cli)
else()
    add_library(os_robotics STATIC "src/os_robotics.cpp")
    target_include_directories(os_robotics PUBLIC "include")
    target_link_libraries(os_robotics PRIVATE os_hal os_core os_cli)
endif()
