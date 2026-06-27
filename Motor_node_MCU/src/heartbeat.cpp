/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Heartbeat Module Implementation
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 */

#include "heartbeat.hpp"
#include "gpio_overlay.hpp"
#include "canfd.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(heartbeat, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * INIT
 * ============================================================
 */

void Heartbeat::init()
{
    led_state_ = false;
    last_toggle_ms_ = k_uptime_get_32();

    GpioOverlay::setHeartbeatLed(false);

    LOG_INF("Heartbeat initialized");
}

/* ============================================================
 * LED TOGGLE
 * ============================================================
 */

void Heartbeat::toggleLed()
{
    led_state_ = !led_state_;

    GpioOverlay::setHeartbeatLed(led_state_);
}

/* ============================================================
 * CAN HEARTBEAT TX
 * ============================================================
 */

void Heartbeat::sendCanHeartbeat()
{
    CanFd::sendHeartbeat();
}

/* ============================================================
 * UPDATE LOOP
 * ============================================================
 */

void Heartbeat::update()
{
    const std::uint32_t now = k_uptime_get_32();

    if ((now - last_toggle_ms_) >= kBlinkIntervalMs) {
        last_toggle_ms_ = now;

        toggleLed();

        sendCanHeartbeat();
    }
}

/* ============================================================
 * END PART 1
 * ============================================================
 *//* ============================================================
 * SAFETY EXTENSION (OPTIONAL WATCHDOG HOOK)
 * ============================================================
 */

namespace
{
    static inline std::uint32_t g_last_heartbeat_ms = 0U;
}

/**
 * @brief Update internal heartbeat timestamp
 */
static void update_heartbeat_timestamp()
{
    g_last_heartbeat_ms = k_uptime_get_32();
}

/* ============================================================
 * ENHANCED UPDATE (FINAL VERSION)
 * ============================================================
 */

void Heartbeat::update()
{
    const std::uint32_t now = k_uptime_get_32();

    if ((now - last_toggle_ms_) >= kBlinkIntervalMs) {
        last_toggle_ms_ = now;

        toggleLed();
        sendCanHeartbeat();

        update_heartbeat_timestamp();
    }
}

/* ============================================================
 * SYSTEM HEALTH CHECK (OPTIONAL EXTENSION)
 * ============================================================
 */

bool heartbeat_is_alive()
{
    const std::uint32_t now = k_uptime_get_32();

    /* If heartbeat hasn't updated for >2 seconds, system is unhealthy */
    constexpr std::uint32_t kTimeoutMs = 2000U;

    return ((now - g_last_heartbeat_ms) <= kTimeoutMs);
}

/* ============================================================
 * DEBUG INFO
 * ============================================================
 */

#ifdef CONFIG_LOG

void heartbeat_debug()
{
    LOG_INF("Heartbeat Debug:");
    LOG_INF("LED state        : %d", Heartbeat::led_state_);
    LOG_INF("Last toggle time : %u", Heartbeat::last_toggle_ms_);
    LOG_INF("Alive status     : %d", heartbeat_is_alive());
}

#endif

/* ============================================================
 * END OF FILE
 * ============================================================
 */