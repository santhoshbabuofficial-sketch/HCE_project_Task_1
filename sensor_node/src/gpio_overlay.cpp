#include "gpio_overlay.hpp"

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sensor_gpio_overlay, LOG_LEVEL_INF);

namespace hce::sensor_node {

bool GpioOverlay::Init() {
    pressure_i2c_bus_ = DEVICE_DT_GET(DT_BUS(DT_ALIAS(pressure_sensor)));
    flow_i2c_bus_ = DEVICE_DT_GET(DT_BUS(DT_ALIAS(flow_sensor)));
    lcd_i2c_bus_ = DEVICE_DT_GET(DT_BUS(DT_ALIAS(lcd_display)));
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(can_bus));
    heartbeat_led_spec_ = GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);

    if (!device_is_ready(pressure_i2c_bus_) || !device_is_ready(flow_i2c_bus_) ||
        !device_is_ready(lcd_i2c_bus_) || !device_is_ready(can_dev_) ||
        !device_is_ready(heartbeat_led_spec_.port)) {
        LOG_ERR("One or more Sensor Node devices are not ready");
        return false;
    }

    const int err = gpio_pin_configure_dt(&heartbeat_led_spec_, GPIO_OUTPUT_INACTIVE);
    if (err != 0) {
        LOG_ERR("Failed to configure heartbeat LED: %d", err);
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

}  // namespace hce::sensor_node
