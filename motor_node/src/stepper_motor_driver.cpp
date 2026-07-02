#include "stepper_motor_driver.hpp"

namespace hce::motor_node {

StepperMotorDriver::StepperMotorDriver(GpioOverlay& overlay) : overlay_(overlay) {}

void StepperMotorDriver::Enable() {
    overlay_.SetMotorEnabled(true);
    enabled_ = true;
}

void StepperMotorDriver::Disable() {
    overlay_.SetMotorEnabled(false);
    enabled_ = false;
}

void StepperMotorDriver::SetDirection(bool forward) {
    overlay_.SetDirection(forward);
}

void StepperMotorDriver::Step() {
    if (enabled_) {
        overlay_.PulseStep();
    }
}

}  // namespace hce::motor_node
