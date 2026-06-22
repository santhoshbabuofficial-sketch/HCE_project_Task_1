#pragma once

#include <cstdint>

namespace sensor_node
{

class GpioOverlay
{
public:
    static bool init();

    static std::uint16_t readPressureAdcRaw();

    static std::uint32_t getFlowPulseCount();

    static std::uint32_t getAndResetFlowPulseCount();

    /*
     * LCD Hardware Functions
     */
    static bool lcdInit();

    static void lcdClear();

    static void lcdSetCursor(
        std::uint8_t row,
        std::uint8_t col);

    static void lcdPrint(
        const char* text);

private:
    GpioOverlay() = delete;
};

} // namespace sensor_node