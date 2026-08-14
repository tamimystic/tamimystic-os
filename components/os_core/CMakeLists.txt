if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_event_bus.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal)
else()
    add_library(os_core STATIC "src/os_event_bus.cpp")
    target_include_directories(os_core PUBLIC "include")
    target_link_libraries(os_core PRIVATE os_hal pthread)
endif()
