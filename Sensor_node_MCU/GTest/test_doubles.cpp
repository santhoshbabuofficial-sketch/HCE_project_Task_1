#include <cstdint>

#include "gpio_overlay.hpp"

namespace sensor_node
{

// ======================================================
// ONLY HARDWARE LAYER MOCKS (GPIO / ADC / CAN PINS)
// ======================================================

// CAN hardware init mock
bool GpioOverlay::canInit()
{
    return true;
}

// CAN transmit mock
bool GpioOverlay::canTransmit(
    std::uint32_t,
    const std::uint8_t*,
    std::uint8_t)
{
    return true;
}

// ADC mock (pressure sensor input)
std::uint16_t GpioOverlay::readPressureAdcRaw()
{
    return 1000;   // fixed test value
}

// Flow pulse counter mock
std::uint32_t GpioOverlay::getAndResetFlowPulseCount()
{
    return 10;     // fixed test value
}

// LCD hardware mocks
bool GpioOverlay::lcdInit()
{
    return true;
}

void GpioOverlay::lcdClear()
{
}

void GpioOverlay::lcdSetCursor(std::uint8_t, std::uint8_t)
{
}

void GpioOverlay::lcdPrint(const char*)
{
}

} // namespace sensor_node