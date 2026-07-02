#include "alarm_logic.hpp"

namespace hce::supervisor_node {

bool AlarmLogic::Evaluate(const AlarmInputs& inputs) const {
    const bool pressure_high = inputs.pressure_valid && (inputs.pressure_pa > kPressureThresholdPa);
    const bool flow_low = inputs.flow_valid && (inputs.flow_lpm < kFlowThresholdLpm);

    // Combined condition: BOTH pressure-high AND flow-low must hold
    // together to raise a sensor-side fault (see file header note).
    const bool combined_sensor_fault = pressure_high && flow_low;

    return combined_sensor_fault || inputs.sensor_heartbeat_faulted ||
           inputs.motor_heartbeat_faulted;
}

}  // namespace hce::supervisor_node
