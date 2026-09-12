if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_audio.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_robotics os_slam os_pnp)
else()
    add_library(os_audio STATIC "src/os_audio.cpp")
    target_include_directories(os_audio PUBLIC "include")
    target_link_libraries(os_audio PRIVATE os_hal os_core os_scheduler os_robotics os_slam os_pnp)
endif()
