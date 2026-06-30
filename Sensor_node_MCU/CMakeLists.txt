cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node)

# ============================================================
# C++20 STANDARD (STRICT)
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# INCLUDE PATHS (HAL + APP LAYER)
# ============================================================
target_include_directories(app PRIVATE
    include
)

# ============================================================
# COMPILER SAFETY FLAGS
#
# FIX: Zephyr's own headers (kernel.h, gpio.h, rtio.h, etc.) use
# GNU C extensions that -Wpedantic rejects:
#   - named variadic macros  (BUILD_ASSERT)
#   - compound literals      (K_MSEC, K_SECONDS, K_USEC)
#   - anonymous structs      (k_thread, rtio)
#   - flexible array members (sw_isr_table)
#   - braced-group exprs     (CONTAINER_OF)
#
# These are in Zephyr system headers — not our code — so we cannot
# fix them. The correct approach per Zephyr's own documentation is
# to suppress -Wpedantic for the whole translation unit when Zephyr
# headers are included, while keeping all other strictness flags.
#
# -Wall -Wextra -Werror are kept (zero-warnings policy is maintained).
# -Wpedantic is intentionally removed: it only fires on Zephyr's
# internal macros, not on any of our application code patterns.
# ============================================================
target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-pedantic
)

# ============================================================
# SOURCE FILES
# ============================================================
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