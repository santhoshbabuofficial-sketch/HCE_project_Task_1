#include <gtest/gtest.h>
#include "canfd.hpp"

TEST(CanFdTest, Init)
{
    EXPECT_TRUE(sensor_node::CanFd::init());
}

TEST(CanFdTest, Heartbeat)
{
    sensor_node::CanFd::sendHeartbeat();
    SUCCEED();
}

TEST(CanFdTest, SensorData)
{
    sensor_node::CanFd::sendSensorData();
    SUCCEED();
}