#include <gtest/gtest.h>
#include "pressure_sensor.hpp"

TEST(PressureSensorTest, Basic)
{
    EXPECT_GE(sensor_node::PressureSensor::readPressureMmHg(), 0);
}