cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node LANGUAGES CXX)

target_sources(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio_overlay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pressure_sensor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/flow_sensor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/heartbeat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lcd_display.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/sensor/src/can_bus_driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/sensor/src/message_codec.cpp
)

# Zero-warnings policy (Rule 3) applies only to our own application code,
# not to Zephyr's kernel/drivers/HAL/CMSIS sources (which are not -Werror
# clean on this toolchain and are not ours to fix).
target_compile_options(app PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wvolatile
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/sensor/include
)
