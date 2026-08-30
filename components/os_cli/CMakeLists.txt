if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_cli.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_pnp os_robotics os_ai os_storage os_apps)
else()
    add_library(os_cli STATIC "src/os_cli.cpp")
    target_include_directories(os_cli PUBLIC "include")
    target_link_libraries(os_cli PRIVATE os_hal os_core os_pnp os_robotics os_ai os_storage os_apps pthread)
endif()
