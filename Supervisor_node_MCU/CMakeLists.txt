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
    src/alarm.cpp
)

# ============================================================
# COMPILER SAFETY FLAGS
# ============================================================
# NOTE on -Wpedantic: intentionally NOT requested. Zephyr 3.7 LTS's own
# public headers (kernel.h, sys_clock.h, sw_isr_table.h, toolchain/gcc.h,
# newlib's stdint.h/string.h, ...) rely on GNU C extensions (compound
# literals, anonymous structs, flexible array members, #include_next,
# named variadic macros) that are not ISO-C++-pedantic-clean. On this
# toolchain (arm-zephyr-eabi-g++, SDK 0.17.2) several of these specific
# diagnostics (-Wvariadic-macros, and two pedantic checks GCC emits with
# a bare "[-Werror]" tag rather than "[-Wpedantic]") do not respond to
# -Wno-error=pedantic, so they cannot be selectively demoted -- the only
# reliable fix is to not enable -Wpedantic in the first place. This is a
# documented limitation of building Zephyr with a strict ISO C++
# frontend, not a defect in application code.
#
# -Wno-error=format: one Zephyr __ASSERT() macro (spinlock.h) passes a
# k_spinlock* where %p expects void*; again upstream, outside app control.
#
# -Wno-error=missing-field-initializers: Zephyr's K_MSGQ_DEFINE() macro
# (Z_MSGQ_INITIALIZER in kernel.h) does not explicitly initialize every
# struct k_msgq field, relying on C99 zero-fill semantics that -Wextra
# flags under C++. Upstream macro behaviour, not application code.
target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-error=format
    -Wno-error=missing-field-initializers
)

# ============================================================
# OPTIONAL: STRICT MISRA-LIKE BUILD HARDENING
# ============================================================
target_compile_options(app PRIVATE
    -fno-exceptions
    -fno-rtti
)