cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sensor_node)

#
# ============================================================
# C++20
# ============================================================
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# ============================================================
# Include Directories
# ============================================================
#
target_include_directories(app PRIVATE
    include
)

#
# ============================================================
# Compiler Warnings
# ============================================================
#
target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

#
# ============================================================
# Source Files
# ============================================================
#
target_sources(app PRIVATE

    src/main.cpp

    src/gpio_overlay.cpp

    src/pressure_sensor.cpp
    src/flow_sensor.cpp

    src/sensor_manager.cpp

    src/lcd_display.cpp

    src/canfd.cpp

    src/heartbeat.cpp
)