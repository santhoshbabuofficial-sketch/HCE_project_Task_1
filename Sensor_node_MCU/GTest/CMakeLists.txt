cmake_minimum_required(VERSION 3.20)

project(sensor_node_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(GTest REQUIRED)

add_compile_options(--coverage -O0 -g)
add_link_options(--coverage)

# =========================
# Sensor Node Library
# =========================
add_library(sensor_node_lib
    ../src/pressure_sensor.cpp
    ../src/flow_sensor.cpp
    ../src/sensor_manager.cpp
    ../src/lcd_Display.cpp
    ../src/canfd.cpp
    ../src/heartbeat.cpp
)

target_include_directories(sensor_node_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/../include
)

# =========================
# Test Executable
# =========================
add_executable(sensor_node_tests
    test_pressure_sensor.cpp
    test_flow_sensor.cpp
    test_sensor_manager.cpp
    test_lcd_Display.cpp
    test_canfd.cpp
    test_heartbeat.cpp
    test_doubles.cpp     # ✅ IMPORTANT: ADD THIS
)

target_link_libraries(sensor_node_tests
    PRIVATE
    sensor_node_lib
    GTest::gtest
    GTest::gtest_main
)

# =========================
# GoogleTest discovery
# =========================
include(GoogleTest)
gtest_discover_tests(sensor_node_tests)

# =========================
# Coverage (optional but fine)
# =========================
add_custom_target(
    coverage
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*'
            --output-file coverage_filtered.info
    COMMAND genhtml coverage_filtered.info --output-directory coverage_report
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS sensor_node_tests
)