/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Heartbeat Module
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Periodic CAN heartbeat transmission
 * - LED blinking (PC4)
 * - System alive indication
 */

#pragma once

#include <cstdint>

namespace motor_node
{

/**
 * @brief Heartbeat manager
 *
 * Sends periodic CAN heartbeat and toggles LED.
 */
class Heartbeat final
{
public:
    Heartbeat() = delete;
    ~Heartbeat() = delete;

    /**
     * @brief Initialize heartbeat module
     */
    static void init();

    /**
     * @brief Periodic update (called from main loop/thread)
     */
    static void update();

private:
    /**
     * @brief Toggle LED state
     */
    static void toggleLed();

    /**
     * @brief Send CAN heartbeat frame
     */
    static void sendCanHeartbeat();

private:
    static inline bool led_state_ = false;

    static inline std::uint32_t last_toggle_ms_ = 0U;

    static constexpr std::uint32_t kBlinkIntervalMs = 500U;
};

} // namespace motor_node