/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * GPIO Overlay Implementation
 * Project: Motor Node MCU
 * Board  : STM32 NUCLEO-G474RE
 * RTOS   : Zephyr 3.7 LTS
 */

#include "gpio_overlay.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(gpio_overlay, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * STATIC MEMBER DEFINITIONS
 * ============================================================
 */

const struct device *GpioOverlay::can_dev_ = nullptr;

gpio_dt_spec GpioOverlay::heartbeat_led_ = GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);

gpio_dt_spec GpioOverlay::stepper_in1_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_in1), gpios);
gpio_dt_spec GpioOverlay::stepper_in2_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_in2), gpios);
gpio_dt_spec GpioOverlay::stepper_in3_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_in3), gpios);
gpio_dt_spec GpioOverlay::stepper_in4_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_in4), gpios);

/* ============================================================
 * INIT FUNCTION
 * ============================================================
 */

bool GpioOverlay::init()
{
    int ret;

    if (!device_is_ready(heartbeat_led_.port)) {
        LOG_ERR("Heartbeat LED GPIO not ready");
        return false;
    }

    ret = gpio_pin_configure_dt(&heartbeat_led_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure heartbeat LED");
        return false;
    }

    if (!device_is_ready(stepper_in1_.port) ||
        !device_is_ready(stepper_in2_.port) ||
        !device_is_ready(stepper_in3_.port) ||
        !device_is_ready(stepper_in4_.port)) {
        LOG_ERR("Stepper GPIO not ready");
        return false;
    }

    ret = gpio_pin_configure_dt(&stepper_in1_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    ret = gpio_pin_configure_dt(&stepper_in2_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    ret = gpio_pin_configure_dt(&stepper_in3_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    ret = gpio_pin_configure_dt(&stepper_in4_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    /* Cache CAN device handle for later retrieval */
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(canfd0));
    if (!device_is_ready(can_dev_)) {
        LOG_ERR("CAN device not ready during GPIO overlay init");
        can_dev_ = nullptr;
        return false;
    }

    LOG_INF("GPIO Overlay initialized successfully");

    return true;
}

/* ============================================================
 * HEARTBEAT LED CONTROL
 * ============================================================
 */

void GpioOverlay::toggleHeartbeatLed()
{
    if (heartbeat_led_.port == nullptr) {
        return;
    }

    gpio_pin_toggle_dt(&heartbeat_led_);
}

void GpioOverlay::setHeartbeatLed(bool enable)
{
    if (heartbeat_led_.port == nullptr) {
        return;
    }

    gpio_pin_set_dt(&heartbeat_led_, static_cast<int>(enable));
}

/* ============================================================
 * STEPPER CONTROL
 * ============================================================
 */

void GpioOverlay::setStepperPins(bool in1, bool in2, bool in3, bool in4)
{
    gpio_pin_set_dt(&stepper_in1_, static_cast<int>(in1));
    gpio_pin_set_dt(&stepper_in2_, static_cast<int>(in2));
    gpio_pin_set_dt(&stepper_in3_, static_cast<int>(in3));
    gpio_pin_set_dt(&stepper_in4_, static_cast<int>(in4));
}

/* ============================================================
 * CAN DEVICE ACCESS
 * ============================================================
 */

const struct device *GpioOverlay::getCanDevice()
{
    return can_dev_;
}

} // namespace motor_node

/* ============================================================
 * RESET OUTPUTS UTILITY (C linkage, safe-state entry)
 * ============================================================
 * Accesses hardware only through the public GpioOverlay API —
 * no private member access.
 */

void gpio_overlay_reset_outputs()
{
    motor_node::GpioOverlay::setHeartbeatLed(false);
    motor_node::GpioOverlay::setStepperPins(false, false, false, false);
}

/* ============================================================
 * DEBUG HELPERS (OPTIONAL)
 * ============================================================
 * Accesses hardware only through the public GpioOverlay API —
 * device readiness is checked via the public accessor.
 */

#ifdef CONFIG_LOG

void gpio_overlay_debug_print()
{
    LOG_INF("GPIO Overlay Debug State:");

    const struct device *can_dev = motor_node::GpioOverlay::getCanDevice();
    LOG_INF("CAN device ready via accessor: %d",
            (can_dev != nullptr) && device_is_ready(can_dev));
}

#endif /* CONFIG_LOG */

/* ============================================================
 * END OF FILE
 * ============================================================
 */