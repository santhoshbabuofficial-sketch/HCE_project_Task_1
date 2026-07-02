/// @file gpio_overlay.hpp
/// @brief Sole layer permitted to reference Device Tree bindings
///        (DT_ALIAS / DT_NODELABEL) for the Sensor Node.
///
/// Per the system data-flow contract:
///   Application code SHALL NEVER bypass gpio_overlay.
///   Application code SHALL NEVER directly access Device Tree.
///   Application code SHALL NEVER directly access STM32 peripherals.
/// Every other class in this project (pressure_sensor, flow_sensor,
/// lcd_display, heartbeat, sensor_manager) talks to hardware exclusively
/// through the methods below.
#pragma once

#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

namespace hce::sensor_node {

/// @brief Thin, mockable wrapper around all Sensor Node DT-bound devices.
class GpioOverlay {
public:
    GpioOverlay() = default;

    /// @brief Resolve and validate all DT-bound devices.
    /// @return true if every required device is ready.
    bool Init();

    /// @brief I2C bus carrying the pressure sensor (LPS22HB).
    const device* PressureI2cBus() const { return pressure_i2c_bus_; }

    /// @brief I2C bus carrying the flow sensor (PAV3015).
    const device* FlowI2cBus() const { return flow_i2c_bus_; }

    /// @brief I2C bus carrying the 16x2 LCD (PCF8574 backpack).
    const device* LcdI2cBus() const { return lcd_i2c_bus_; }

    /// @brief CAN-FD controller device (FDCAN1).
    const device* CanDevice() const { return can_dev_; }

    /// @brief Drive the heartbeat LED (PC4).
    /// @param on true = LED on.
    /// @return true on success.
    bool SetHeartbeatLed(bool on);

    static constexpr uint16_t kPressureSensorI2cAddr = 0x5CU;  ///< LPS22HB.
    static constexpr uint16_t kFlowSensorI2cAddr = 0x40U;      ///< PAV3015.
    static constexpr uint16_t kLcdI2cAddr = 0x27U;             ///< PCF8574.

private:
    const device* pressure_i2c_bus_ = nullptr;
    const device* flow_i2c_bus_ = nullptr;
    const device* lcd_i2c_bus_ = nullptr;
    const device* can_dev_ = nullptr;
    const device* heartbeat_led_dev_ = nullptr;
    gpio_dt_spec heartbeat_led_spec_{};
    bool initialized_ = false;
};

}  // namespace hce::sensor_node
