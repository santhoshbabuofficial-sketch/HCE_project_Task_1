cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(supervisor_node LANGUAGES CXX)

target_sources(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gpio_overlay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/heartbeat_monitor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/alarm_logic.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/supervisor_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/supervisor/src/can_bus_driver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/supervisor/src/message_codec.cpp
)

# Strict warnings apply only to our own application code, not to
# Zephyr's kernel/drivers/HAL/CMSIS sources (which are not -Werror clean
# on this toolchain and are not ours to fix).
target_compile_options(app PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wvolatile
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../common/supervisor/include
)
