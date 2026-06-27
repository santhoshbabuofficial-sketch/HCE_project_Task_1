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

K_THREAD_STACK_DEFINE(motor_stack, 1024);
K_THREAD_STACK_DEFINE(heartbeat_stack, 1024);

/* ============================================================
 * THREAD CONTROL BLOCKS
 * ============================================================
 */

static struct k_thread motor_thread_data;
static struct k_thread heartbeat_thread_data;

/* ============================================================
 * MOTOR THREAD
 * ============================================================
 */

static void motor_thread(void *, void *, void *)
{
    while (1) {
        StepperMotorDriver::update();
    }
}

/* ============================================================
 * HEARTBEAT THREAD
 * ============================================================
 */

static void heartbeat_thread(void *, void *, void *)
{
    while (1) {
        Heartbeat::update();
        k_msleep(10);
    }
}

/* ============================================================
 * CAN RX CALLBACK (BRIDGE FROM ISR / DRIVER)
 * ============================================================
 */

static void can_rx_bridge(const struct device *dev,
                          struct can_frame *frame,
                          void *user_data)
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
    Heartbeat::init();

    LOG_INF("System initialization complete");

    return ok;
}

/* ============================================================
 * CAN FILTER SETUP
 * ============================================================
 */

static void can_setup_rx()
{
    const struct device *can_dev = DEVICE_DT_GET(DT_ALIAS(canfd0));

    if (!device_is_ready(can_dev)) {
        LOG_ERR("CAN device not ready in RX setup");
        return;
    }

    can_add_rx_filter(can_dev,
                      can_rx_bridge,
                      nullptr,
                      nullptr,
                      nullptr,
                      CAN_FILTER_IDE_MASK);

    LOG_INF("CAN RX filter configured");
}

/* ============================================================
 * MAIN
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

    LOG_INF("System running");

    while (1) {
        k_msleep(1000);
    }

    return 0;
}

} // namespace motor_node
/* ============================================================
 * SYSTEM FAULT / EMERGENCY STOP HANDLING
 * ============================================================
 */

namespace
{
    static inline bool g_system_fault = false;
}

/**
 * @brief Trigger safe shutdown of motor system
 */
static void system_emergency_stop()
{
    g_system_fault = true;

    StepperMotorDriver::stop();
    GpioOverlay::setHeartbeatLed(false);

    LOG_ERR("EMERGENCY STOP ACTIVATED");
}

/* ============================================================
 * WATCHDOG / SAFETY MONITOR (OPTIONAL EXTENSION)
 * ============================================================
 */

static void system_health_monitor()
{
    /* Placeholder for future expansion:
     * - CAN timeout detection
     * - motor stall detection
     * - heartbeat validation
     */

    if (g_system_fault) {
        while (1) {
            k_msleep(1000);
        }
    }
}

/* ============================================================
 * CLEAN SHUTDOWN HANDLER
 * ============================================================
 */

static void system_shutdown()
{
    LOG_WRN("System shutdown requested");

    StepperMotorDriver::stop();
    GpioOverlay::setStepperPins(false, false, false, false);
    GpioOverlay::setHeartbeatLed(false);
}

/* ============================================================
 * EXTENDED MAIN LOOP (FINAL CONTROL LOOP)
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

    /* Start threads */
    k_thread_create(&motor_thread_data,
                    motor_stack,
                    K_THREAD_STACK_SIZEOF(motor_stack),
                    motor_thread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    k_thread_create(&heartbeat_thread_data,
                    heartbeat_stack,
                    K_THREAD_STACK_SIZEOF(heartbeat_stack),
                    heartbeat_thread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    LOG_INF("System fully operational");

    /* ========================================================
     * MAIN SAFETY LOOP
     * ========================================================
     */

    while (1) {

        system_health_monitor();

        /* Simulated fault hook (future CAN or ISR trigger) */
        if (false) {
            system_emergency_stop();
        }

        k_msleep(100);
    }

    system_shutdown();

    return 0;
}

/* ============================================================
 * END OF FILE
 * ============================================================
 */