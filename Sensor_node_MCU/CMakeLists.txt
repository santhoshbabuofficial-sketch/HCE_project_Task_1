cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sensor_node)

target_include_directories(app PRIVATE
    include
)

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