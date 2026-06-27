#include "lcd_Display.hpp"

#include "gpio_overlay.hpp"
#include "sensor_manager.hpp"

#include <array>
#include <cstdint>
#include <cstdio>

namespace
{

// ============================================================
// LCD STATE FLAG
// ============================================================

bool g_estop_active = false;

} // namespace

namespace sensor_node
{

bool LcdDisplay::init() noexcept
{
    return GpioOverlay::lcdInit();
}

void LcdDisplay::update() noexcept
{
    // If E-STOP active → freeze normal display
    if (g_estop_active)
    {
        return;
    }

    const std::uint16_t pressure =
        SensorManager::getPressureMmHg();

    const std::uint16_t flow =
        SensorManager::getFlowMlMin();

    std::array<char, 17> line1 {};
    std::array<char, 17> line2 {};

    (void)std::snprintf(
        line1.data(),
        line1.size(),
        "P:%u mmHg",
        static_cast<unsigned>(pressure));

    (void)std::snprintf(
        line2.data(),
        line2.size(),
        "F:%u mL/min",
        static_cast<unsigned>(flow));

    GpioOverlay::lcdClear();

    GpioOverlay::lcdSetCursor(0U, 0U);
    GpioOverlay::lcdPrint(line1.data());

    GpioOverlay::lcdSetCursor(1U, 0U);
    GpioOverlay::lcdPrint(line2.data());
}

void LcdDisplay::showMessage(const char* text) noexcept
{
    if (text == nullptr)
    {
        return;
    }

    g_estop_active = true;

    GpioOverlay::lcdClear();
    GpioOverlay::lcdSetCursor(0U, 0U);
    GpioOverlay::lcdPrint(text);
}

} // namespace sensor_node