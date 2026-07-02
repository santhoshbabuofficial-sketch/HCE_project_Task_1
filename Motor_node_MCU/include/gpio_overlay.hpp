/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * GPIO Overlay Abstraction Layer
 * Project: Motor Node MCU
 * Board  : STM32 NUCLEO-G474RE
 * RTOS   : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Only hardware access layer (GPIO + CAN handle exposure if needed)
 * - No business logic
 */

#pragma once

#include <cstdint>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/can.h>

namespace motor_node
{

/**
 * @brief Hardware abstraction wrapper for GPIO and CAN access.
 *
 * Static-only utility class. Instantiation is explicitly forbidden
 * because all members are static (no per-instance state).
 */
class GpioOverlay final
{
public:
    /// @brief Deleted: static-only utility class, no instantiation allowed.
    GpioOverlay() = delete;

    /// @brief Deleted: static-only utility class.
    ~GpioOverlay() = delete;

    /**
     * @brief Initialize all GPIO peripherals.
     * @return true if initialization successful, false otherwise.
     */
    static bool init();

    /**
     * @brief Toggle heartbeat LED.
     */
    static void toggleHeartbeatLed();

    /**
     * @brief Set heartbeat LED state.
     * @param enable true = ON, false = OFF
     */
    static void setHeartbeatLed(bool enable);

    /**
     * @brief Drive the TMC2209 STEP pin.
     * @param level true = STEP pin driven high, false = low.
     */
    static void setStepperStep(bool level);

    /**
     * @brief Drive the TMC2209 DIR pin.
     * @param forward true = forward rotation, false = reverse.
     */
    static void setStepperDir(bool forward);

    /**
     * @brief Drive the TMC2209 ENABLE pin.
     *
     * The devicetree alias for this pin is GPIO_ACTIVE_LOW (matching
     * the TMC2209's hardware-active-low EN input), so callers always
     * use normal positive logic here: true = driver outputs enabled,
     * false = driver outputs disabled (coils free / de-energized).
     *
     * @param enable true = enable driver outputs, false = disable.
     */
    static void setStepperEnable(bool enable);

    /**
     * @brief Get CAN device handle (FDCAN1).
     * @return pointer to Zephyr CAN device
     */
    static const struct device *getCanDevice();

private:
    static const struct device *can_dev_;

    static gpio_dt_spec heartbeat_led_;

    static gpio_dt_spec stepper_step_;
    static gpio_dt_spec stepper_dir_;
    static gpio_dt_spec stepper_enable_;
};

} // namespace motor_node