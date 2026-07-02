cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node LANGUAGES CXX)

# Zero-warnings policy (Rule 3). -Wpedantic is intentionally omitted at the
# global level because Zephyr's own headers use GNU extensions; it is
# re-enabled per translation unit in application code where clean.
zephyr_compile_options(
  -Wall
  -Wextra
  -Werror
  -Wvolatile
)

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

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/sensor/include
)
