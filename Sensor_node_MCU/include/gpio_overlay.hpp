#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Hardware abstraction layer.
 *
 * Owns all low-level peripheral access:
 * ADC, GPIO, I2C, CAN-FD.
 *
 * All methods are static (singleton-like HAL).
 * No heap allocation; all state held in translation-unit
 * anonymous namespace of gpio_overlay.cpp.
 */
class GpioOverlay final
{
public:

    // ============================================================
    // SYSTEM INIT
    // ============================================================

    /**
     * @brief Initialize all hardware peripherals.
     *
     * Must be called once before any other GpioOverlay method.
     *
     * @return true  All peripherals initialised successfully.
     * @return false At least one peripheral failed to initialise.
     */
    [[nodiscard]] static bool init() noexcept;

    // ============================================================
    // PRESSURE SENSOR (ADC1 – PA0 – Channel 1)
    // ============================================================

    /**
     * @brief Read raw ADC sample from the pressure sensor.
     *
     * @return 12-bit ADC sample (0–4095). Returns 0 on read error.
     */
    [[nodiscard]] static std::uint16_t readPressureAdcRaw() noexcept;

    // ============================================================
    // FLOW SENSOR (PC2 – GPIO interrupt)
    // ============================================================

    /**
     * @brief Return the current flow pulse count without clearing it.
     *
     * @return Number of rising-edge pulses counted since last reset.
     */
    [[nodiscard]] static std::uint32_t getFlowPulseCount() noexcept;

    /**
     * @brief Atomically read and clear the flow pulse counter.
     *
     * @return Number of rising-edge pulses since the previous call.
     */
    [[nodiscard]] static std::uint32_t getAndResetFlowPulseCount() noexcept;

    // ============================================================
    // LCD DISPLAY (I2C1 – PCF8574 backpack)
    // ============================================================

    /**
     * @brief Initialise the LCD controller via I2C.
     *
     * @return true  LCD responded and was configured.
     * @return false I2C device not ready or write failed.
     */
    [[nodiscard]] static bool lcdInit() noexcept;

    /**
     * @brief Send the clear-display command and wait 2 ms.
     */
    static void lcdClear() noexcept;

    /**
     * @brief Move the LCD cursor to the specified row and column.
     *
     * @param row  Display row (0 = first line, 1 = second line).
     * @param col  Display column (0-based).
     */
    static void lcdSetCursor(
        std::uint8_t row,
        std::uint8_t col) noexcept;

    /**
     * @brief Write a null-terminated string to the LCD at current cursor.
     *
     * @param text Pointer to null-terminated string. Must not be nullptr.
     */
    static void lcdPrint(const char* text) noexcept;

    // ============================================================
    // CAN-FD (FDCAN1 – PA11 RX / PA12 TX)
    // ============================================================

    /**
     * @brief Initialise FDCAN1, attach RX filter, and start the controller.
     *
     * @return true  Controller started successfully.
     * @return false Device not ready, filter add failed, or start failed.
     */
    [[nodiscard]] static bool canInit() noexcept;

    /**
     * @brief Transmit a CAN frame.
     *
     * @param id     Standard CAN ID (11-bit).
     * @param data   Pointer to payload bytes. Must not be nullptr.
     * @param length Number of payload bytes (1–8 for classic, up to 64 for FD).
     * @return true  Frame accepted by the driver.
     * @return false Null pointer, zero length, or driver error.
     */
    [[nodiscard]] static bool canTransmit(
        std::uint32_t    id,
        const std::uint8_t* data,
        std::uint8_t     length) noexcept;

    // ============================================================
    // HEARTBEAT LED (PC4 – Active HIGH)
    // ============================================================

    /**
     * @brief Pulse the heartbeat LED on PC4.
     *
     * Drives PC4 HIGH for 500 ms then LOW.
     * Blocks the calling thread for the pulse duration.
     */
    static void heartbeatLedPulse() noexcept;

private:

    /// @cond PRIVATE — static-only utility class; construction forbidden.
    GpioOverlay()  = delete;
    ~GpioOverlay() = delete;

    GpioOverlay(const GpioOverlay&)            = delete;
    GpioOverlay& operator=(const GpioOverlay&) = delete;
    /// @endcond
};

} // namespace sensor_node