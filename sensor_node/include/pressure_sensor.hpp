/// @file pressure_sensor.hpp
/// @brief Driver for the LPS22HB absolute-pressure sensor (I2C2).
#pragma once

#include <cstdint>

#include "gpio_overlay.hpp"

namespace hce::sensor_node {

/// @brief Interface so PressureSensor can be mocked on the host build.
class IPressureSensor {
public:
    virtual ~IPressureSensor() = default;
    virtual bool Init() = 0;
    virtual bool Read(float& pressure_pa_out) = 0;
};

/// @brief LPS22HB pressure sensor driver.
class PressureSensor final : public IPressureSensor {
public:
    explicit PressureSensor(GpioOverlay& overlay);

    bool Init() override;

    /// @brief Read absolute pressure.
    /// @param pressure_pa_out Receives the pressure in pascals.
    /// @return true on a successful, ACKed I2C transaction.
    bool Read(float& pressure_pa_out) override;

private:
    // LPS22HB register map (ST datasheet, DocID028669).
    static constexpr uint8_t kRegWhoAmI = 0x0FU;
    static constexpr uint8_t kWhoAmIValue = 0xB1U;
    static constexpr uint8_t kRegCtrl1 = 0x10U;
    static constexpr uint8_t kCtrl1OdrOneHz = 0x10U;
    static constexpr uint8_t kRegPressOutXl = 0x28U;  ///< Auto-increment MSB set externally.
    static constexpr uint8_t kAutoIncrementBit = 0x80U;
    static constexpr float kPressureLsbToHpa = 1.0F / 4096.0F;
    static constexpr float kHpaToPa = 100.0F;

    GpioOverlay& overlay_;
    bool initialized_ = false;
};

}  // namespace hce::sensor_node
