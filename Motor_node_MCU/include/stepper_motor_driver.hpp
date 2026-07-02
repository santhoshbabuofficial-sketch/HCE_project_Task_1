/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stepper Motor Driver (TMC2209, STEP/DIR control)
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - STEP/DIR pulse generation for a TMC2209 driver
 * - Start/Stop control from CAN commands
 * - Speed control as a percentage of the configured speed range
 */

#pragma once

#include <cstdint>

namespace motor_node
{

/**
 * @brief Stepper Motor Driver
 *
 * Drives a TMC2209 in STEP/DIR mode (UART interface not used).
 * No hardware access directly (uses GpioOverlay only).
 */
class StepperMotorDriver final
{
public:
    /// @brief Deleted: static-only utility class, no instantiation allowed.
    StepperMotorDriver() = delete;

    /// @brief Deleted: static-only utility class.
    ~StepperMotorDriver() = delete;

    /**
     * @brief Initialize motor driver state.
     */
    static void init();

    /**
     * @brief Start continuous rotation.
     *
     * Enables the TMC2209 driver outputs and begins pulsing STEP
     * at the rate set by the current speed percentage.
     */
    static void start();

    /**
     * @brief Stop motor immediately and disable driver outputs.
     */
    static void stop();

    /**
     * @brief Periodic update function (called from motor thread).
     */
    static void update();

    /**
     * @brief Set rotation speed as a percentage of the configured range.
     *
     * 0%   -> kMaxStepDelayMs between pulses (slowest)
     * 100% -> kMinStepDelayMs between pulses (fastest)
     *
     * @param percent Desired speed, clamped to [0, 100].
     */
    static void setSpeedPercent(std::uint8_t percent);

    /**
     * @brief Set rotation direction.
     * @param forward true = forward, false = reverse.
     */
    static void setDirection(bool forward);

private:
    /**
     * @brief Emit a single STEP pulse on the TMC2209 STEP pin.
     */
    static void step();

    /**
     * @brief Compute the current inter-step delay from speed_percent_.
     * @return Delay in milliseconds between STEP pulses.
     */
    static std::uint32_t stepDelayMs();

private:
    static inline bool          running_       = false;
    static inline bool          direction_     = true;

    /// @brief Current speed setting, 0-100 (%). Defaults to 50%.
    static inline std::uint8_t  speed_percent_ = 50U;

    /// @brief Width of the STEP high pulse, in microseconds
    ///        (TMC2209 requires a minimum STEP high time; see datasheet).
    static constexpr std::uint32_t kStepPulseWidthUs = 2U;

    /// @brief Inter-step delay at 100% speed (fastest).
    static constexpr std::uint32_t kMinStepDelayMs = 2U;

    /// @brief Inter-step delay at 0% speed (slowest, still moving).
    static constexpr std::uint32_t kMaxStepDelayMs = 20U;
};

} // namespace motor_node