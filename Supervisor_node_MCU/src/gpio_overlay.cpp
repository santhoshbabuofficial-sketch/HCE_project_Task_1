#include "gpio_overlay.hpp"

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

/* ============================================================
 * STATIC MEMBER DEFINITIONS
 * ============================================================
 */

const struct device* GpioOverlay::gpio_port_ = nullptr;

gpio_dt_spec GpioOverlay::alarm_led_spec_ = GPIO_DT_SPEC_GET_OR(DT_ALIAS(alarm_led), gpios, {0});

bool GpioOverlay::initialized_ = false;

/* ============================================================
 * PUBLIC API
 * ============================================================
 */

bool GpioOverlay::Init()
{
    if (initialized_)
    {
        return true;
    }

    gpio_port_ = alarm_led_spec_.port;

    if (!device_is_ready(gpio_port_))
    {
        return false;
    }

    int ret = gpio_pin_configure_dt(&alarm_led_spec_, GPIO_OUTPUT_INACTIVE);
    if (ret != 0)
    {
        return false;
    }

    /* Ensure LED starts OFF */
    gpio_pin_set_dt(&alarm_led_spec_, 0);

    initialized_ = true;
    return true;
}

void GpioOverlay::AlarmLedOn()
{
    if (!initialized_)
    {
        return;
    }

    gpio_pin_set_dt(&alarm_led_spec_, 1);
}

void GpioOverlay::AlarmLedOff()
{
    if (!initialized_)
    {
        return;
    }

    gpio_pin_set_dt(&alarm_led_spec_, 0);
}

void GpioOverlay::AlarmLedToggle()
{
    if (!initialized_)
    {
        return;
    }

    static bool state = false;
    state = !state;

    gpio_pin_set_dt(&alarm_led_spec_, static_cast<int>(state));
}