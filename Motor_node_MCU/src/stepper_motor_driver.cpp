/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Stepper Motor Driver Implementation
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
 * INTERNAL FULL-STEP SEQUENCE
 * ============================================================
 * Sequence for L298N (2-phase ON full-step)
 *
 * Step 0: 1 0 1 0
 * Step 1: 0 1 1 0
 * Step 2: 0 1 0 1
 * Step 3: 1 0 0 1
 *
 * Each row = IN1 IN2 IN3 IN4
 * ============================================================
 */

static constexpr bool kStepSequence[4][4] =
{
    { true,  false, true,  false }, // step 0
    { false, true,  true,  false }, // step 1
    { false, true,  false, true  }, // step 2
    { true,  false, false, true  }  // step 3
};

/* ============================================================
 * STATE VARIABLES
 * ============================================================
 */

static bool g_initialized = false;

/* ============================================================
 * INIT
 * ============================================================
 */

void StepperMotorDriver::init()
{
    running_ = false;
    step_index_ = 0U;

    g_initialized = true;

    LOG_INF("StepperMotorDriver initialized");
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

    running_ = true;

    LOG_INF("Stepper motor START");
}

/* ============================================================
 * STOP MOTOR
 * ============================================================
 */

void StepperMotorDriver::stop()
{
    running_ = false;

    /* Immediately disable coils */
    GpioOverlay::setStepperPins(false, false, false, false);

    LOG_INF("Stepper motor STOP");
}

/* ============================================================
 * STEP EXECUTION
 * ============================================================
 */

void StepperMotorDriver::step()
{
    const bool *pattern = kStepSequence[step_index_];

    GpioOverlay::setStepperPins(
        pattern[0],
        pattern[1],
        pattern[2],
        pattern[3]
    );

    step_index_ = (step_index_ + 1U) & 0x03U;
}

/* ============================================================
 * UPDATE LOOP
 * ============================================================
 */

void StepperMotorDriver::update()
{
    if (!running_) {
        return;
    }

    step();

    k_msleep(kStepDelayMs);
}

/* ============================================================
 * END PART 1
 * ============================================================
 */
/* ============================================================
 * TIMING & SAFETY EXTENSIONS
 * ============================================================
 */

namespace
{
    /**
     * @brief Safety watchdog flag (future extension hook)
     *
     * If motor is left running too long without CAN refresh,
     * this can be used to auto-stop (not enabled yet).
     */
    static inline std::uint32_t g_last_step_time_ms = 0U;
}

/* ============================================================
 * TIMESTAMP UPDATE HOOK (OPTIONAL)
 * ============================================================
 */

static void update_timestamp()
{
    g_last_step_time_ms = k_uptime_get_32();
}

/* ============================================================
 * EXTENDED STEP FUNCTION (SAFE VERSION HOOK)
 * ============================================================
 */

void StepperMotorDriver::step()
{
    const bool *pattern = kStepSequence[step_index_];

    GpioOverlay::setStepperPins(
        pattern[0],
        pattern[1],
        pattern[2],
        pattern[3]
    );

    step_index_ = (step_index_ + 1U) & 0x03U;

    update_timestamp();
}

/* ============================================================
 * IDLE PROTECTION (OPTIONAL SAFETY FEATURE)
 * ============================================================
 */

void StepperMotorDriver::update()
{
    if (!running_) {
        return;
    }

    /* Execute one step */
    step();

    /* Delay controls speed */
    k_msleep(kStepDelayMs);
}

/* ============================================================
 * EMERGENCY STOP HOOK (FUTURE CAN FAULT USE)
 * ============================================================
 */

void stepper_motor_emergency_stop()
{
    StepperMotorDriver::stop();

    GpioOverlay::setStepperPins(false, false, false, false);
}

/* ============================================================
 * DEBUG HELPERS
 * ============================================================
 */

#ifdef CONFIG_LOG

void stepper_motor_debug()
{
    LOG_INF("Stepper Motor Debug:");
    LOG_INF("Running state: %d", StepperMotorDriver::running_);
    LOG_INF("Step index   : %u", StepperMotorDriver::step_index_);
}

#endif

/* ============================================================
 * END OF FILE
 * ============================================================
 */