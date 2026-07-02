/// @file stepper_motor_driver.hpp
/// @brief TMC2209 STEP/DIR/ENABLE driver for the NEMA-11 stepper motor.
#pragma once

#include <cstdint>

#include "gpio_overlay.hpp"

namespace hce::motor_node {

/// @brief Interface so StepperMotorDriver can be mocked on the host build.
class IStepperMotorDriver {
public:
    virtual ~IStepperMotorDriver() = default;
    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual void SetDirection(bool forward) = 0;
    virtual void Step() = 0;
    virtual bool IsEnabled() const = 0;
};

/// @brief Concrete TMC2209 STEP/DIR/ENABLE driver.
class StepperMotorDriver final : public IStepperMotorDriver {
public:
    explicit StepperMotorDriver(GpioOverlay& overlay);

    void Enable() override;
    void Disable() override;
    void SetDirection(bool forward) override;
    void Step() override;
    bool IsEnabled() const override { return enabled_; }

private:
    GpioOverlay& overlay_;
    bool enabled_ = false;
};

}  // namespace hce::motor_node
