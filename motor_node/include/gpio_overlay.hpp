/// @file gpio_overlay.hpp
/// @brief Sole layer permitted to reference Device Tree bindings for the
///        Motor Node. See sensor_node/include/gpio_overlay.hpp for the
///        architectural contract (application code never touches DT or
///        STM32 peripherals directly).
#pragma once

#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace hce::motor_node {

/// @brief Thin, mockable wrapper around all Motor Node DT-bound devices.
class GpioOverlay {
public:
    GpioOverlay() = default;

    /// @brief Resolve and validate all required DT-bound devices.
    bool Init();

    /// @brief CAN-FD controller device (FDCAN1).
    const device* CanDevice() const { return can_dev_; }

    /// @brief Drive the heartbeat LED (PC4).
    bool SetHeartbeatLed(bool on);

    /// @brief Pulse the STEP line (PB11) high then low.
    bool PulseStep();

    /// @brief Set rotation direction (PB12). true = forward.
    bool SetDirection(bool forward);

    /// @brief Enable/disable the TMC2209 driver (PB13, active-low ENABLE).
    /// @param enabled true = motor enabled (EN pin driven low).
    bool SetMotorEnabled(bool enabled);

private:
    const device* can_dev_ = nullptr;
    gpio_dt_spec heartbeat_led_spec_{};
    gpio_dt_spec step_spec_{};
    gpio_dt_spec dir_spec_{};
    gpio_dt_spec enable_spec_{};
    bool initialized_ = false;
};

}  // namespace hce::motor_node
