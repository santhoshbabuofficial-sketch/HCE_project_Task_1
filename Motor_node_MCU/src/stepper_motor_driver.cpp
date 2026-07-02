/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stepper Motor Driver Implementation (TMC2209, STEP/DIR mode)
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 */

#include "stepper_motor_driver.hpp"
#include "gpio_overlay.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(stepper_motor, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * STATE VARIABLES
 * ============================================================
 */

static bool g_initialized = false;

/* ============================================================
 * SAFETY WATCHDOG TIMESTAMP
 * ============================================================
 * Tracks last step time for future idle-protection extension.
 */

static std::uint32_t g_last_step_time_ms = 0U;

/* ============================================================
 * INIT
 * ============================================================
 */

void StepperMotorDriver::init()
{
    running_       = false;
    direction_     = true;
    speed_percent_ = 50U;

    g_initialized = true;

    GpioOverlay::setStepperDir(direction_);

    LOG_INF("StepperMotorDriver initialized (TMC2209, speed=%u%%)", speed_percent_);
}

/* ============================================================
 * START MOTOR
 * ============================================================
 */

void StepperMotorDriver::start()
{
    if (!g_initialized) {
        init();
    }

    GpioOverlay::setStepperDir(direction_);
    GpioOverlay::setStepperEnable(true);

    running_ = true;

    LOG_INF("Stepper motor START (TMC2209 enabled, speed=%u%%)", speed_percent_);
}

/* ============================================================
 * STOP MOTOR
 * ============================================================
 */

void StepperMotorDriver::stop()
{
    running_ = false;

    /* Immediately disable driver outputs and drop STEP low */
    GpioOverlay::setStepperEnable(false);
    GpioOverlay::setStepperStep(false);

    LOG_INF("Stepper motor STOP (TMC2209 disabled)");
}

/* ============================================================
 * SPEED CONTROL
 * ============================================================
 */

void StepperMotorDriver::setSpeedPercent(std::uint8_t percent)
{
    speed_percent_ = (percent > 100U) ? 100U : percent;

    LOG_INF("Stepper speed set to %u%%", speed_percent_);
}

std::uint32_t StepperMotorDriver::stepDelayMs()
{
    const std::uint32_t range = kMaxStepDelayMs - kMinStepDelayMs;

    return kMaxStepDelayMs - ((range * speed_percent_) / 100U);
}

/* ============================================================
 * DIRECTION CONTROL
 * ============================================================
 */

void StepperMotorDriver::setDirection(bool forward)
{
    direction_ = forward;

    GpioOverlay::setStepperDir(direction_);
}

/* ============================================================
 * STEP EXECUTION
 * ============================================================
 * Emits one STEP pulse: STEP high for kStepPulseWidthUs, then low.
 * The TMC2209 latches a new microstep on the rising edge; DIR is
 * held constant while pulsing. Single definition — includes
 * timestamp update for the optional safety watchdog extension
 * (Rule: no duplicate defs).
 * ============================================================
 */

void StepperMotorDriver::step()
{
    GpioOverlay::setStepperStep(true);
    k_busy_wait(kStepPulseWidthUs);
    GpioOverlay::setStepperStep(false);

    /* Update watchdog timestamp */
    g_last_step_time_ms = k_uptime_get_32();
}

/* ============================================================
 * UPDATE LOOP
 * ============================================================
 * Single definition — called from motor thread.
 * ============================================================
 */

void StepperMotorDriver::update()
{
    if (!running_) {
        return;
    }

    step();

    k_msleep(stepDelayMs());
}

} // namespace motor_node

/* ============================================================
 * EMERGENCY STOP HOOK (C linkage, for CAN fault path)
 * ============================================================
 */

void stepper_motor_emergency_stop()
{
    motor_node::StepperMotorDriver::stop();

    /* Redundant disable/STEP-low for safety */
    motor_node::GpioOverlay::setStepperEnable(false);
    motor_node::GpioOverlay::setStepperStep(false);
}

/* ============================================================
 * DEBUG HELPERS
 * ============================================================
 * Note: running_, direction_, speed_percent_ are private; debug
 * info is logged inside the class methods. External debug only
 * reports the initialized flag (which is file-scope, not private).
 * ============================================================
 */

#ifdef CONFIG_LOG

void stepper_motor_debug()
{
    LOG_INF("Stepper Motor Debug: (detailed state visible in init/start/stop logs)");
}

#endif /* CONFIG_LOG */

/* ============================================================
 * END OF FILE
 * ============================================================
 */