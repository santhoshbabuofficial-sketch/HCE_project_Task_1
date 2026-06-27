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
 */
class GpioOverlay final
{
public:
    GpioOverlay() = default;
    ~GpioOverlay() = default;

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
     * @brief Set stepper motor GPIO pins (full step control).
     * @param in1 IN1 state
     * @param in2 IN2 state
     * @param in3 IN3 state
     * @param in4 IN4 state
     */
    static void setStepperPins(bool in1, bool in2, bool in3, bool in4);

    /**
     * @brief Get CAN device handle (FDCAN1).
     * @return pointer to Zephyr CAN device
     */
    static const struct device *getCanDevice();

private:
    static const struct device *can_dev_;

    static gpio_dt_spec heartbeat_led_;

    static gpio_dt_spec stepper_in1_;
    static gpio_dt_spec stepper_in2_;
    static gpio_dt_spec stepper_in3_;
    static gpio_dt_spec stepper_in4_;
};

} // namespace motor_node