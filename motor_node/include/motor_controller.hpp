/// @file motor_controller.hpp
/// @brief Interprets RUN (0x400) / STOP (0x100) CAN commands and drives
///        the stepper motor accordingly. Implements the State design
///        pattern (Rule 4: named design pattern) over {STOPPED, RUNNING}.
#pragma once

#include <cstdint>

#include "stepper_motor_driver.hpp"

namespace hce::motor_node {

/// @brief Motor Node run/stop state machine (State pattern).
enum class MotorState : uint8_t {
    kStopped = 0U,
    kRunning = 1U,
};

/// @brief Coordinates CAN commands with the stepper driver.
class MotorController {
public:
    explicit MotorController(IStepperMotorDriver& driver);

    /// @brief Handle a RUN command (CAN ID 0x400).
    void OnRunCommand(uint32_t step_rate_hz, bool forward);

    /// @brief Handle a STOP command (CAN ID 0x100). Immediately disables
    ///        the driver (safety-critical path).
    void OnStopCommand();

    /// @brief Generate STEP pulses at the configured rate. Intended to be
    ///        called periodically from the motor step thread; internally
    ///        no-ops unless MotorState::kRunning.
    void Tick();

    MotorState State() const { return state_; }

private:
    IStepperMotorDriver& driver_;
    MotorState state_ = MotorState::kStopped;
    uint32_t step_rate_hz_ = 0U;
    uint32_t tick_counter_ = 0U;
    static constexpr uint32_t kTickPeriodUs = 1000U;  ///< Tick() call period.
};

}  // namespace hce::motor_node
