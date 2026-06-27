#
# Motor Node MCU
# STM32 NUCLEO-G474RE
# Zephyr RTOS 3.7 LTS
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(motor_node_mcu)

# ============================================================
# Include directories
# ============================================================

target_include_directories(app PRIVATE
    include
)

# ============================================================
# Source files
# ============================================================

target_sources(app PRIVATE
    src/main.cpp
    src/gpio_overlay.cpp
    src/canfd.cpp
    src/heartbeat.cpp
    src/stepper_motor_driver.cpp
)