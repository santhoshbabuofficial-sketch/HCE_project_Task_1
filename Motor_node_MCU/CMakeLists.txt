#
# Motor Node MCU
# STM32 NUCLEO-G474RE
# Zephyr RTOS 3.7 LTS
#

cmake_minimum_required(VERSION 3.20.0)

# ============================================================
# Devicetree Overlay
#
# nucleo_g474re.overlay lives in boards/nucleo_g474re.overlay,
# which is Zephyr's standard per-board overlay location. West
# auto-detects and applies an overlay at
# <app_root>/boards/<board>.overlay with zero extra CMake
# configuration — no DTC_OVERLAY_FILE override needed here.
# If the board target ever changes, rename the overlay file to
# match the new board name under boards/.
# ============================================================

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(motor_node_mcu)

# ============================================================
# C++20 Standard (Rule 2)
# ============================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# Compiler Flags — Zero Warnings Policy for application code
# (Rule 2 & 3)
#
# NOTE: -Wpedantic is intentionally NOT applied. Zephyr's own
# kernel headers (spinlock.h, kernel.h, sys_clock.h, log_msg.h,
# sw_isr_table.h, etc.) rely on GNU C extensions — flexible
# array members, compound literals, anonymous structs, and
# statement expressions — that are required by the Zephyr C
# kernel ABI and are not valid strict ISO C++. These are
# vendor-RTOS headers outside this project's control; -Wpedantic
# cannot be satisfied against them in a C++ Zephyr application.
# This is consistent with upstream Zephyr's own C++ sample apps.
#
# -Wno-format silences a known Zephyr __ASSERT false positive
# where %p is used against typed pointers (e.g. k_spinlock*)
# inside spinlock.h; this is a vendor header issue, not an
# application defect.
#
# -Wno-volatile silences errors from ST's CMSIS/HAL/LL register
# access macros (SET_BIT, CLEAR_BIT, SCB->SHCSR |= ..., etc. in
# stm32g4xx_ll_*.h and mpu_armv7.h). These macros perform
# compound assignment on volatile-qualified memory-mapped
# hardware registers, which C++20 deprecates and GCC's
# -Wvolatile flags as an error under -Werror. This is required,
# correct register-access behavior for direct hardware control
# and is vendor silicon-support code outside this project's
# control; the application's own files contain no volatile
# compound-assignment usage of this kind.
# ============================================================

target_compile_options(app PRIVATE
    -std=c++20
    -Wall
    -Wextra
    -Werror
    -Wno-format
    -Wno-volatile
)

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