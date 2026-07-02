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

gpio_dt_spec GpioOverlay::stepper_step_   = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_step), gpios);
gpio_dt_spec GpioOverlay::stepper_dir_    = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_dir), gpios);
gpio_dt_spec GpioOverlay::stepper_enable_ = GPIO_DT_SPEC_GET(DT_ALIAS(stepper_enable), gpios);

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

    if (!device_is_ready(stepper_step_.port) ||
        !device_is_ready(stepper_dir_.port) ||
        !device_is_ready(stepper_enable_.port)) {
        LOG_ERR("TMC2209 GPIO not ready");
        return false;
    }

    ret = gpio_pin_configure_dt(&stepper_step_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    ret = gpio_pin_configure_dt(&stepper_dir_, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) { return false; }

    /* INACTIVE here means "not enabled" thanks to GPIO_ACTIVE_LOW
     * on the stepper-enable alias, so the driver boots disabled. */
    ret = gpio_pin_configure_dt(&stepper_enable_, GPIO_OUTPUT_INACTIVE);
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
 * TMC2209 STEP/DIR/ENABLE CONTROL
 * ============================================================
 */

void GpioOverlay::setStepperStep(bool level)
{
    if (stepper_step_.port == nullptr) {
        return;
    }

    gpio_pin_set_dt(&stepper_step_, static_cast<int>(level));
}

void GpioOverlay::setStepperDir(bool forward)
{
    if (stepper_dir_.port == nullptr) {
        return;
    }

    gpio_pin_set_dt(&stepper_dir_, static_cast<int>(forward));
}

void GpioOverlay::setStepperEnable(bool enable)
{
    if (stepper_enable_.port == nullptr) {
        return;
    }

    /* stepper_enable_ is GPIO_ACTIVE_LOW in the devicetree, so
     * gpio_pin_set_dt() already handles the active-low inversion:
     * passing 1 here drives the physical EN pin low (enabled). */
    gpio_pin_set_dt(&stepper_enable_, static_cast<int>(enable));
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
    motor_node::GpioOverlay::setStepperEnable(false);
    motor_node::GpioOverlay::setStepperStep(false);
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