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
    led_state_      = false;
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
 * Single definition — includes watchdog timestamp update
 * for optional safety extension (Rule: no duplicate defs).
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

} // namespace motor_node

/* ============================================================
 * SAFETY WATCHDOG TIMESTAMP (file-scope, not private member)
 * ============================================================
 */

namespace
{
    static std::uint32_t g_last_heartbeat_ms = 0U;
}

/**
 * @brief Returns true if heartbeat has fired within the last 2 seconds.
 * @return true if system heartbeat is alive.
 */
bool heartbeat_is_alive()
{
    const std::uint32_t now = k_uptime_get_32();

    constexpr std::uint32_t kTimeoutMs = 2000U;

    return ((now - g_last_heartbeat_ms) <= kTimeoutMs);
}

/* ============================================================
 * DEBUG INFO
 * ============================================================
 * led_state_ and last_toggle_ms_ are private members of
 * Heartbeat — they are not directly accessible here.
 * Debug logs the watchdog status instead (file-scope data).
 * ============================================================
 */

#ifdef CONFIG_LOG

void heartbeat_debug()
{
    LOG_INF("Heartbeat Debug:");
    LOG_INF("Alive status: %d", heartbeat_is_alive());
}

#endif /* CONFIG_LOG */

/* ============================================================
 * END OF FILE
 * ============================================================
 */