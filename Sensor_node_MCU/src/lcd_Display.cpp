#include "lcd_display.hpp"

#include "gpio_overlay.hpp"
#include "sensor_manager.hpp"

#include <array>
#include <cstdint>
#include <cstdio>

namespace sensor_node
{

bool
LcdDisplay::init()
{
    return GpioOverlay::lcdInit();
}

void
LcdDisplay::update()
{
    const std::uint16_t pressure =
        SensorManager::getPressureMmHg();

    const std::uint16_t flow =
        SensorManager::getFlowMlMin();

    std::array<char, 17> line1 {};
    std::array<char, 17> line2 {};

    (void)std::snprintf(
        line1.data(),
        line1.size(),
        "Pressure=%u",
        static_cast<unsigned>(pressure));

    (void)std::snprintf(
        line2.data(),
        line2.size(),
        "Flow=%u",
        static_cast<unsigned>(flow));

    GpioOverlay::lcdClear();

    GpioOverlay::lcdSetCursor(
        0U,
        0U);

    GpioOverlay::lcdPrint(
        line1.data());

    GpioOverlay::lcdSetCursor(
        1U,
        0U);

    GpioOverlay::lcdPrint(
        line2.data());
}

} // namespace sensor_node