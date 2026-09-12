if(DEFINED ENV{IDF_PATH})
    idf_component_register(SRCS "src/os_espnow.cpp"
                           INCLUDE_DIRS "include"
                           PRIV_REQUIRES os_hal os_core os_scheduler os_robotics os_slam esp_wifi esp_netif esp_event nvs_flash)
else()
    add_library(os_espnow STATIC "src/os_espnow.cpp")
    target_include_directories(os_espnow PUBLIC "include")
    target_link_libraries(os_espnow PRIVATE os_hal os_core os_scheduler os_robotics os_slam)
endif()
