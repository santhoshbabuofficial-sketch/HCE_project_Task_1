/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stepper Motor Driver (Full-Step Control)
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Full-step sequence generation
 * - Start/Stop control from CAN commands
 * - Timing control (speed via kStepDelayMs)
 */

#pragma once

#include <cstdint>

namespace motor_node
{

/**
 * @brief Stepper Motor Driver
 *
 * Controls L298N driver in full-step mode.
 * No hardware access directly (uses GpioOverlay only).
 */
class StepperMotorDriver final
{
public:
    StepperMotorDriver() = delete;
    ~StepperMotorDriver() = delete;

    /**
     * @brief Initialize motor driver state
     */
    static void init();

    /**
     * @brief Start continuous rotation
     */
    static void start();

    /**
     * @brief Stop motor immediately
     */
    static void stop();

    /**
     * @brief Periodic update function (called from main loop)
     */
    static void update();

private:
    /**
     * @brief Execute next step in sequence
     */
    static void step();

private:
    static inline bool running_ = false;

    static inline std::uint8_t step_index_ = 0U;

    static constexpr std::uint32_t kStepDelayMs = 10U;
};

} // namespace motor_node