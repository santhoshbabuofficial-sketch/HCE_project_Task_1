#include <gtest/gtest.h>
#include "flow_sensor.hpp"

TEST(FlowSensorTest, Basic)
{
    EXPECT_GE(sensor_node::FlowSensor::readFlowMlMin(), 0);
}