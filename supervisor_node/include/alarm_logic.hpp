/// @file alarm_logic.hpp
/// @brief Combined pressure/flow/heartbeat fault evaluation.
///
/// Fault condition (per system safety spec):
///   (Pressure > kPressureThresholdPa AND Flow < kFlowThresholdLpm)
///   OR ThreeConsecutiveSensorHeartbeatsMissed
///   OR ThreeConsecutiveMotorHeartbeatsMissed
///
/// Note: pressure-high and flow-low are evaluated as a COMBINED condition
/// (both must be true together), not as two independent single-sensor
/// faults - this reflects a deliberate safety-logic update from
/// independent-fault evaluation to a combined-fault evaluation.
#pragma once

#include <cstdint>

namespace hce::supervisor_node {

/// @brief Inputs the alarm evaluation needs on any given tick.
struct AlarmInputs {
    float pressure_pa;
    bool pressure_valid;
    float flow_lpm;
    bool flow_valid;
    bool sensor_heartbeat_faulted;
    bool motor_heartbeat_faulted;
};

/// @brief Stateless fault-evaluation strategy (Strategy pattern: the
///        evaluation rule is isolated behind Evaluate() so it can be
///        swapped/tested independently of CAN transport or GPIO).
class AlarmLogic {
public:
    static constexpr float kPressureThresholdPa = 1025.0F;
    static constexpr float kFlowThresholdLpm = 60.0F;

    /// @brief Evaluate the combined fault condition.
    /// @return true if a fault (STOP condition) is present.
    bool Evaluate(const AlarmInputs& inputs) const;
};

}  // namespace hce::supervisor_node
