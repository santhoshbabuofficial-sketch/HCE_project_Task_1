#include "gpio_overlay.hpp"

#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(motor_gpio_overlay, LOG_LEVEL_INF);

namespace hce::motor_node {

bool GpioOverlay::Init() {
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(can_bus));
    heartbeat_led_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);
    step_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_step), gpios);
    dir_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_dir), gpios);
    enable_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_enable), gpios);

    if (!device_is_ready(can_dev_) || !device_is_ready(heartbeat_led_spec_.port) ||
        !device_is_ready(step_spec_.port) || !device_is_ready(dir_spec_.port) ||
        !device_is_ready(enable_spec_.port)) {
        LOG_ERR("One or more Motor Node devices are not ready");
        return false;
    }

    int err = gpio_pin_configure_dt(&heartbeat_led_spec_, GPIO_OUTPUT_INACTIVE);
    err |= gpio_pin_configure_dt(&step_spec_, GPIO_OUTPUT_INACTIVE);
    err |= gpio_pin_configure_dt(&dir_spec_, GPIO_OUTPUT_INACTIVE);
    // Boot in the disabled (safe) state: ENABLE is active-low, so
    // "inactive" here drives EN high, i.e. the TMC2209 stays disabled.
    err |= gpio_pin_configure_dt(&enable_spec_, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("GPIO configuration failed: %d", err);
        return false;
    }

    initialized_ = true;
    return true;
}

bool GpioOverlay::SetHeartbeatLed(bool on) {
    if (!initialized_) {
        return false;
    }
    return gpio_pin_set_dt(&heartbeat_led_spec_, on ? 1 : 0) == 0;
}

bool GpioOverlay::PulseStep() {
    if (!initialized_) {
        return false;
    }
    gpio_pin_set_dt(&step_spec_, 1);
    gpio_pin_set_dt(&step_spec_, 0);
    return true;
}

bool GpioOverlay::SetDirection(bool forward) {
    if (!initialized_) {
        return false;
    }
    return gpio_pin_set_dt(&dir_spec_, forward ? 1 : 0) == 0;
}

bool GpioOverlay::SetMotorEnabled(bool enabled) {
    if (!initialized_) {
        return false;
    }
    // gpio_dt_spec with GPIO_ACTIVE_LOW already inverts logical vs
    // physical level, so "1" here means "logically enabled".
    return gpio_pin_set_dt(&enable_spec_, enabled ? 1 : 0) == 0;
}

}  // namespace hce::motor_node
