/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * gpio_overlay.hpp
 *
 * Hardware Abstraction Layer (HAL)
 * for the Sensor Node.
 *
 * Board:
 *     STM32 NUCLEO-G474RE
 *
 * RTOS:
 *     Zephyr 3.7
 *
 * Language:
 *     C++20
 *
 * Description:
 *     Provides hardware access for:
 *
 *     - ADC Pressure Sensor
 *     - GPIO Flow Sensor
 *     - I2C LCD
 *     - FDCAN
 *
 * This class hides all Zephyr driver APIs from the
 * application layer.
 */

#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Hardware abstraction layer.
 *
 * This class owns all low-level hardware access.
 *
 * Higher layers shall never directly access
 * Zephyr GPIO, ADC, I2C or CAN drivers.
 */
class GpioOverlay final
{
public:

    /**
     * @brief Initialize all hardware peripherals.
     *
     * Initializes:
     * - ADC
     * - GPIO Interrupt
     *
     * @retval true  Initialization successful.
     * @retval false Initialization failed.
     */
    [[nodiscard]]
    static bool init() noexcept;

    /*==========================================================
     * Pressure Sensor
     *==========================================================*/

    /**
     * @brief Read raw ADC value.
     *
     * Reads ADC1 Channel 1.
     *
     * @return Raw 12-bit ADC value.
     */
    [[nodiscard]]
    static std::uint16_t
    readPressureAdcRaw() noexcept;

    /*==========================================================
     * Flow Sensor
     *==========================================================*/

    /**
     * @brief Get current pulse counter.
     *
     * @return Current pulse count.
     */
    [[nodiscard]]
    static std::uint32_t
    getFlowPulseCount() noexcept;

    /**
     * @brief Get pulse count and reset counter.
     *
     * @return Pulse count since last reset.
     */
    [[nodiscard]]
    static std::uint32_t
    getAndResetFlowPulseCount() noexcept;

    /*==========================================================
     * LCD
     *==========================================================*/

    /**
     * @brief Initialize LCD.
     *
     * @retval true Success.
     * @retval false Failure.
     */
    [[nodiscard]]
    static bool
    lcdInit() noexcept;

    /**
     * @brief Clear LCD display.
     */
    static void
    lcdClear() noexcept;

    /**
     * @brief Set LCD cursor.
     *
     * @param row LCD row.
     * @param col LCD column.
     */
    static void
    lcdSetCursor(
        std::uint8_t row,
        std::uint8_t col) noexcept;

    /**
     * @brief Print string to LCD.
     *
     * @param text Null terminated string.
     */
    static void
    lcdPrint(
        const char* text) noexcept;

    /*==========================================================
     * CAN
     *==========================================================*/

    /**
     * @brief Initialize CAN controller.
     *
     * @retval true Success.
     * @retval false Failure.
     */
    [[nodiscard]]
    static bool
    canInit() noexcept;

    /**
     * @brief Transmit CAN frame.
     *
     * @param id CAN Identifier.
     * @param data Payload pointer.
     * @param length Payload length.
     *
     * @retval true Frame transmitted.
     * @retval false Transmission failed.
     */
    [[nodiscard]]
    static bool
    canTransmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length) noexcept;

private:

    GpioOverlay() = delete;
    ~GpioOverlay() = delete;

    GpioOverlay(
        const GpioOverlay&) = delete;

    GpioOverlay& operator=(
        const GpioOverlay&) = delete;

    GpioOverlay(
        GpioOverlay&&) = delete;

    GpioOverlay& operator=(
        GpioOverlay&&) = delete;
};

} // namespace sensor_node