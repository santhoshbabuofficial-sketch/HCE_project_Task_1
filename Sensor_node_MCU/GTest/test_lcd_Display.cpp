#include <gtest/gtest.h>
#include "lcd_Display.hpp"

TEST(LcdDisplayTest, Init)
{
    EXPECT_TRUE(sensor_node::LcdDisplay::init());
}

TEST(LcdDisplayTest, Update)
{
    sensor_node::LcdDisplay::update();
    SUCCEED();
}

TEST(LcdDisplayTest, ShowMessage)
{
    sensor_node::LcdDisplay::showMessage("TEST");
    SUCCEED();
}