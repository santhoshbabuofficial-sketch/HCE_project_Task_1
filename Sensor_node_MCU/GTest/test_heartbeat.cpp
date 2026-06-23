#include <gtest/gtest.h>
#include "heartbeat.hpp"

TEST(HeartbeatTest, Init)
{
    EXPECT_TRUE(sensor_node::Heartbeat::init());
}

TEST(HeartbeatTest, Update)
{
    sensor_node::Heartbeat::update();
    SUCCEED();
}