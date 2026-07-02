#include "motor_controller.hpp"

namespace hce::motor_node {

MotorController::MotorController(IStepperMotorDriver& driver) : driver_(driver) {}

void MotorController::OnRunCommand(uint32_t step_rate_hz, bool forward) {
    driver_.SetDirection(forward);
    driver_.Enable();
    step_rate_hz_ = step_rate_hz;
    tick_counter_ = 0U;
    state_ = MotorState::kRunning;
}

void MotorController::OnStopCommand() {
    // Safety-critical path: disable immediately regardless of current state.
    driver_.Disable();
    step_rate_hz_ = 0U;
    state_ = MotorState::kStopped;
}

void MotorController::Tick() {
    if (state_ != MotorState::kRunning || step_rate_hz_ == 0U) {
        return;
    }

    const uint32_t ticks_per_step = 1000000U / (step_rate_hz_ * kTickPeriodUs);
    const uint32_t divisor = (ticks_per_step == 0U) ? 1U : ticks_per_step;

    ++tick_counter_;
    if (tick_counter_ >= divisor) {
        tick_counter_ = 0U;
        driver_.Step();
    }
}

}  // namespace hce::motor_node
