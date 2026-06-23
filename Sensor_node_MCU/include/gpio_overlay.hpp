#pragma once

#include <cstdint>

namespace sensor_node
{

class GpioOverlay
{
public:
    /*
     * ==========================================================
     * Initialization
     * ==========================================================
     */
    static bool init();

    /*
     * ==========================================================
     * Pressure Sensor Hardware Access
     * PA0 -> ADC1_IN1
     * ==========================================================
     */
    static std::uint16_t readPressureAdcRaw();

    /*
     * ==========================================================
     * Flow Sensor Hardware Access
     * PC2 -> GPIO Interrupt
     * ==========================================================
     */
    static std::uint32_t getFlowPulseCount();

    static std::uint32_t getAndResetFlowPulseCount();

    /*
     * ==========================================================
     * LCD Hardware Access
     * I2C1
     * PB8 -> SCL
     * PB9 -> SDA
     * ==========================================================
     */
    static bool lcdInit();

    static void lcdClear();

    static void lcdSetCursor(
        std::uint8_t row,
        std::uint8_t col);

    static void lcdPrint(
        const char* text);

    /*
     * ==========================================================
     * CAN Hardware Access
     * FDCAN1
     * PA11 -> RX
     * PA12 -> TX
     * ==========================================================
     */
   static bool canInit();

static bool canTransmit(
    std::uint32_t id,
    const std::uint8_t* data,
    std::uint8_t length);

private:
    GpioOverlay() = delete;
};

} // namespace sensor_node