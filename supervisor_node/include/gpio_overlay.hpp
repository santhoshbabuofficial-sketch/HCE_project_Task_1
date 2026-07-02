/// @file gpio_overlay.hpp
/// @brief Sole layer permitted to reference Device Tree bindings for the
///        Supervisor Node.
#pragma once

#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace hce::supervisor_node {

/// @brief Thin, mockable wrapper around all Supervisor Node DT-bound devices.
class GpioOverlay {
public:
    GpioOverlay() = default;

    bool Init();

    /// @brief CAN-FD controller device (FDCAN1).
    const device* CanDevice() const { return can_dev_; }

    /// @brief Drive the alarm LED (PC4).
    bool SetAlarmLed(bool on);

private:
    const device* can_dev_ = nullptr;
    gpio_dt_spec alarm_led_spec_{};
    bool initialized_ = false;
};

}  // namespace hce::supervisor_node
