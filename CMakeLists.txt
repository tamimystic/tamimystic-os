cmake_minimum_required(VERSION 3.16)

# Detect if we are building within ESP-IDF
if(DEFINED ENV{IDF_PATH})
    message(STATUS "ESP-IDF Environment detected. Building for target.")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(tamimystic_os)
else()
    message(STATUS "No ESP-IDF Environment detected. Building for Native POSIX Simulation.")
    project(tamimystic_os C CXX)
    
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_C_STANDARD 11)

    # Force static linking so we don't need libstdc++-6.dll
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

    # Define a custom preprocessor flag to indicate native build
    add_compile_definitions(OS_TARGET_NATIVE)
    add_compile_definitions(_WIN32_WINNT=0x0A00)

    # Manually collect components for native build
    add_subdirectory(components/os_hal)
    add_subdirectory(components/os_core)
    add_subdirectory(components/os_cli)
    add_subdirectory(components/os_network)
    add_subdirectory(components/os_config)
    add_subdirectory(components/os_storage)
    add_subdirectory(components/os_web)
    add_subdirectory(components/os_apps)
    add_subdirectory(components/os_robotics)
    add_subdirectory(components/os_ai)
    add_subdirectory(main)
endif()
