if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_apps.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_cli os_storage)
else()
    add_library(os_apps STATIC "src/os_apps.cpp")
    target_include_directories(os_apps PUBLIC "include")
    target_link_libraries(os_apps PRIVATE os_hal os_core os_cli os_storage)
endif()
