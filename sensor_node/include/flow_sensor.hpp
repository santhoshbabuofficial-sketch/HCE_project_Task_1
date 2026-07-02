/// @file flow_sensor.hpp
/// @brief Driver for the PAV3015 air-velocity sensor (I2C3), converted
///        to volumetric air flow in litres/minute.
#pragma once

#include <cstdint>

#include "gpio_overlay.hpp"

namespace hce::sensor_node {

/// @brief Interface so FlowSensor can be mocked on the host build.
class IFlowSensor {
public:
    virtual ~IFlowSensor() = default;
    virtual bool Init() = 0;
    virtual bool Read(float& flow_lpm_out) = 0;
};

/// @brief PAV3015 air-velocity sensor driver with velocity -> flow
///        conversion (Q = v * A).
class FlowSensor final : public IFlowSensor {
public:
    explicit FlowSensor(GpioOverlay& overlay);

    bool Init() override;

    /// @brief Read air flow.
    /// @param flow_lpm_out Receives flow in litres per minute.
    /// @return true on a successful, ACKed I2C transaction.
    bool Read(float& flow_lpm_out) override;

private:
    static constexpr uint8_t kRegVelocityMsb = 0x00U;
    static constexpr uint8_t kRegVelocityLsb = 0x01U;
    static constexpr float kVelocityLsbToMps = 0.001F;  ///< mm/s -> m/s.

    /// @brief Effective duct cross-sectional area (m^2) used for the
    ///        velocity -> volumetric-flow conversion, per calibration.
    static constexpr float kDuctAreaM2 = 0.0007854F;  // 10 mm diameter duct.
    static constexpr float kM3PerSecToLitrePerMin = 60000.0F;

    GpioOverlay& overlay_;
    bool initialized_ = false;
};

}  // namespace hce::sensor_node
