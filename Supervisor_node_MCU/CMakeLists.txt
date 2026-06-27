cmake_minimum_required(VERSION 3.20.0)

# ============================================================
# Zephyr base
# ============================================================
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(supervisor_node_mcu LANGUAGES C CXX)

# ============================================================
# C++ STANDARD
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# INCLUDE PATHS
# ============================================================
target_include_directories(app PRIVATE
    include
)

# ============================================================
# SOURCE FILES
# ============================================================
target_sources(app PRIVATE
    src/main.cpp
    src/gpio_overlay.cpp
    src/canfd.cpp
    src/heartbeat_monitoring.cpp
)

# ============================================================
# COMPILER SAFETY FLAGS
# ============================================================
target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wpedantic
)

# ============================================================
# OPTIONAL: STRICT MISRA-LIKE BUILD HARDENING
# ============================================================
target_compile_options(app PRIVATE
    -fno-exceptions
    -fno-rtti
)