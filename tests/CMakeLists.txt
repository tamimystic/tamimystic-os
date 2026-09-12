cmake_minimum_required(VERSION 3.16)

add_executable(tamimystic_tests
    test_main.cpp
    test_kinematics.cpp
    test_arm_ik.cpp
    test_slam_nav.cpp
    test_espnow_swarm.cpp
    test_ble.cpp
    test_audio_dsp.cpp
    test_event_bus.cpp
    test_python_runner.cpp
)

target_include_directories(tamimystic_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/components/os_hal/include
    ${CMAKE_SOURCE_DIR}/components/os_core/include
    ${CMAKE_SOURCE_DIR}/components/os_scheduler/include
    ${CMAKE_SOURCE_DIR}/components/os_robotics/include
    ${CMAKE_SOURCE_DIR}/components/os_slam/include
    ${CMAKE_SOURCE_DIR}/components/os_audio/include
    ${CMAKE_SOURCE_DIR}/components/os_espnow/include
    ${CMAKE_SOURCE_DIR}/components/os_ble/include
    ${CMAKE_SOURCE_DIR}/components/os_pnp/include
    ${CMAKE_SOURCE_DIR}/components/os_storage/include
    ${CMAKE_SOURCE_DIR}/components/os_config/include
    ${CMAKE_SOURCE_DIR}/components/os_apps/include
)

target_link_libraries(tamimystic_tests PRIVATE
    os_hal
    os_core
    os_scheduler
    os_robotics
    os_slam
    os_audio
    os_espnow
    os_ble
    os_pnp
    os_storage
    os_config
    os_apps
)

if(WIN32)
    target_link_libraries(tamimystic_tests PRIVATE ws2_32)
endif()

enable_testing()
add_test(NAME TamimysticUnitTestSuite COMMAND tamimystic_tests)
