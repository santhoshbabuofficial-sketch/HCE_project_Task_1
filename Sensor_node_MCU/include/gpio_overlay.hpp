#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Hardware abstraction layer.
 *
 * Owns all low-level peripheral access:
 * ADC, GPIO, I2C, CAN.
 */
class GpioOverlay final
{
public:

    /**
     * @brief Initialize all hardware peripherals.
     *
     * @return true if success, false otherwise
     */
    [[nodiscard]] static bool init() noexcept;

    // ============================================================
    // PRESSURE SENSOR (ADC)
    // ============================================================

    /**
     * @brief Read raw ADC value from pressure sensor.
     *
     * @return 12-bit ADC sample value
     */
    [[nodiscard]] static std::uint16_t readPressureAdcRaw() noexcept;

    // ============================================================
    // FLOW SENSOR (GPIO PULSE)
    // ============================================================

    /**
     * @brief Get current flow pulse count.
     *
     * @return pulse count
     */
    [[nodiscard]] static std::uint32_t getFlowPulseCount() noexcept;

    /**
     * @brief Get and reset flow pulse counter.
     *
     * @return pulses since last read
     */
    [[nodiscard]] static std::uint32_t getAndResetFlowPulseCount() noexcept;

    // ============================================================
    // LCD (I2C)
    // ============================================================

    static bool lcdInit() noexcept;
    static void lcdClear() noexcept;

    static void lcdSetCursor(
        std::uint8_t row,
        std::uint8_t col) noexcept;

    static void lcdPrint(
        const char* text) noexcept;

    // ============================================================
    // CAN (FDCAN)
    // ============================================================

    static bool canInit() noexcept;

    static bool canTransmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length) noexcept;

    // ============================================================
    // HEARTBEAT LED (PC4)
    // ============================================================

    /**
     * @brief Pulse heartbeat LED (PC4).
     *
     * Active HIGH:
     * ON for 500ms then OFF.
     */
    static void heartbeatLedPulse() noexcept;

private:

    GpioOverlay() = delete;
    ~GpioOverlay() = delete;

    GpioOverlay(const GpioOverlay&) = delete;
    GpioOverlay& operator=(const GpioOverlay&) = delete;
};

} // namespace sensor_node