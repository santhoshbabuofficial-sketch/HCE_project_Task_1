#include "gpio_overlay.hpp"

#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(supervisor_gpio_overlay, LOG_LEVEL_INF);

namespace hce::supervisor_node {

bool GpioOverlay::Init() {
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(can_bus));
    alarm_led_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(alarm_led), gpios);

    if (!device_is_ready(can_dev_) || !device_is_ready(alarm_led_spec_.port)) {
        LOG_ERR("One or more Supervisor Node devices are not ready");
        return false;
    }

    const int err = gpio_pin_configure_dt(&alarm_led_spec_, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("Failed to configure alarm LED: %d", err);
        return false;
    }

    initialized_ = true;
    return true;
}

bool GpioOverlay::SetAlarmLed(bool on) {
    if (!initialized_) {
        return false;
    }
    return gpio_pin_set_dt(&alarm_led_spec_, on ? 1 : 0) == 0;
}

}  // namespace hce::supervisor_node
