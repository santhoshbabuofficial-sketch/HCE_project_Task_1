/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Main Application Entry
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <cstring>

#include "gpio_overlay.hpp"
#include "canfd.hpp"
#include "heartbeat.hpp"
#include "stepper_motor_driver.hpp"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * THREAD STACKS
 * ============================================================
 */

K_THREAD_STACK_DEFINE(motor_stack,     1024);
K_THREAD_STACK_DEFINE(heartbeat_stack, 1024);

/* ============================================================
 * THREAD CONTROL BLOCKS
 * ============================================================
 */

static struct k_thread motor_thread_data;
static struct k_thread heartbeat_thread_data;

/* ============================================================
 * SYSTEM FAULT FLAG
 * ============================================================
 */

static bool g_system_fault = false;

/* ============================================================
 * MOTOR THREAD
 * ============================================================
 */

static void motor_thread(void *, void *, void *)
{
    while (true) {
        StepperMotorDriver::update();
    }
}

/* ============================================================
 * HEARTBEAT THREAD
 * ============================================================
 */

static void heartbeat_thread(void *, void *, void *)
{
    while (true) {
        Heartbeat::update();
        k_msleep(10);
    }
}

/* ============================================================
 * CAN RX CALLBACK (registered once in can_setup_rx)
 * ============================================================
 */

static void can_rx_bridge(const struct device *dev,
                          struct can_frame    *frame,
                          void                *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(user_data);

    if (frame == nullptr) {
        return;
    }

    CanFd::processReceivedMessage(frame);
}

/* ============================================================
 * SYSTEM INIT
 * ============================================================
 */

static bool system_init()
{
    bool ok = true;

    ok &= GpioOverlay::init();
    ok &= CanFd::init();

    StepperMotorDriver::init();
    StepperMotorDriver::setSpeedPercent(50U); /* TMC2209 default: 50% speed */
    Heartbeat::init();

    LOG_INF("System initialization complete");

    return ok;
}

/* ============================================================
 * CAN FILTER SETUP
 * ============================================================
 * Zephyr can_add_rx_filter() signature:
 *   int can_add_rx_filter(const struct device *dev,
 *                         can_rx_callback_t    cb,
 *                         void                *user_data,
 *                         const struct can_filter *filter);
 *
 * Pass filter = nullptr to accept all frames (open filter).
 * ============================================================
 */

static void can_setup_rx()
{
    const struct device *can_dev = DEVICE_DT_GET(DT_ALIAS(canfd0));

    if (!device_is_ready(can_dev)) {
        LOG_ERR("CAN device not ready in RX setup");
        return;
    }

    /* Open filter: accept all frames.
     * Zero-initialize first, then set fields explicitly. This
     * avoids -Werror=missing-field-initializers / designator-order
     * errors that occur with brace-list designated initializers
     * if the struct has more members than {flags, id, mask} in a
     * given Zephyr version/configuration. */
    static struct can_filter s_accept_all_filter;
    memset(&s_accept_all_filter, 0, sizeof(s_accept_all_filter));
    s_accept_all_filter.flags = 0U;
    s_accept_all_filter.id    = 0U;
    s_accept_all_filter.mask  = 0U;

    int ret = can_add_rx_filter(can_dev,
                                can_rx_bridge,
                                nullptr,
                                &s_accept_all_filter);
    if (ret < 0) {
        LOG_ERR("CAN RX filter registration failed: %d", ret);
        return;
    }

    LOG_INF("CAN RX filter configured");
}

/* ============================================================
 * EMERGENCY STOP
 * ============================================================
 */

static void system_emergency_stop()
{
    g_system_fault = true;

    StepperMotorDriver::stop();
    GpioOverlay::setHeartbeatLed(false);

    LOG_ERR("EMERGENCY STOP ACTIVATED");
}

/* ============================================================
 * HEALTH MONITOR (called from main safety loop)
 * ============================================================
 */

static void system_health_monitor()
{
    /* Placeholder for future extension:
     * - CAN timeout detection
     * - motor stall detection
     * - heartbeat validation
     */

    if (g_system_fault) {
        while (true) {
            k_msleep(1000);
        }
    }
}

/* ============================================================
 * CLEAN SHUTDOWN
 * ============================================================
 */

static void system_shutdown()
{
    LOG_WRN("System shutdown requested");

    StepperMotorDriver::stop();
    GpioOverlay::setStepperEnable(false);
    GpioOverlay::setStepperStep(false);
    GpioOverlay::setHeartbeatLed(false);
}

/* ============================================================
 * MAIN — single definition
 * ============================================================
 */

int main()
{
    LOG_INF("Motor Node MCU Booting...");

    if (!system_init()) {
        LOG_ERR("System init failed");
        return -1;
    }

    can_setup_rx();

    /* Start motor thread */
    k_thread_create(&motor_thread_data,
                    motor_stack,
                    K_THREAD_STACK_SIZEOF(motor_stack),
                    motor_thread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    /* Start heartbeat thread */
    k_thread_create(&heartbeat_thread_data,
                    heartbeat_stack,
                    K_THREAD_STACK_SIZEOF(heartbeat_stack),
                    heartbeat_thread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    LOG_INF("System fully operational");

    /* ========================================================
     * MAIN SAFETY LOOP
     * ======================================================== */
    while (true) {

        system_health_monitor();

        /* Fault injection hook (driven by CAN or ISR in future) */
        if (false) {
            system_emergency_stop();
        }

        k_msleep(100);
    }

    system_shutdown();

    return 0;
}

} // namespace motor_node

/* ============================================================
 * END OF FILE
 * ============================================================
 */