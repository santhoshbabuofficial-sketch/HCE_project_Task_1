#ifndef GPIO_OVERLAY_HPP
#define GPIO_OVERLAY_HPP

/**
 * ============================================================
 * GPIO OVERLAY (HARDWARE ABSTRACTION LAYER)
 * ============================================================
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Abstract all GPIO operations
 * - Control ALARM LED (PC4)
 * - Provide safe hardware API for other modules
 *
 * RULE:
 * - No direct hardware access outside this layer
 * ============================================================
 */

#include <cstdint>
#include <zephyr/drivers/gpio.h>

class GpioOverlay
{
public:
    /**
     * @brief Initialize all GPIO peripherals
     * @return true if initialization successful
     */
    static bool Init();

    /**
     * @brief Turn ON alarm LED (PC4)
     */
    static void AlarmLedOn();

    /**
     * @brief Turn OFF alarm LED (PC4)
     */
    static void AlarmLedOff();

    /**
     * @brief Toggle alarm LED (heartbeat blink)
     */
    static void AlarmLedToggle();

private:
    static constexpr std::uint8_t kAlarmLedPin = 4U;

    static const struct device* gpio_port_;

    static gpio_dt_spec alarm_led_spec_;

    static bool initialized_;
};

#endif // GPIO_OVERLAY_HPP