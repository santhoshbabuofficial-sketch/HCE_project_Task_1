#include <gtest/gtest.h>
#include "sensor_manager.hpp"

TEST(SensorManagerTest, Update)
{
    sensor_node::SensorManager::update();
    SUCCEED();
}