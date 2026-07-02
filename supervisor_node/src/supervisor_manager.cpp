#include "supervisor_manager.hpp"

namespace hce::supervisor_node {

SupervisorManager::SupervisorManager(GpioOverlay& overlay, hce::supervisor::ICanBusDriver& can_bus)
    : overlay_(overlay), can_bus_(can_bus) {}

void SupervisorManager::OnPressureReceived(const hce::supervisor::MessageCodec::Payload& payload) {
    const hce::supervisor::PressureMessage msg = hce::supervisor::MessageCodec::DecodePressure(payload);
    latest_pressure_pa_ = msg.pressure_pa;
    pressure_valid_ = true;
}

void SupervisorManager::OnFlowReceived(const hce::supervisor::MessageCodec::Payload& payload) {
    const hce::supervisor::FlowMessage msg = hce::supervisor::MessageCodec::DecodeFlow(payload);
    latest_flow_lpm_ = msg.flow_lpm;
    flow_valid_ = true;
}

void SupervisorManager::OnSensorHeartbeatReceived() {
    heartbeat_monitor_.OnSensorHeartbeatReceived();
}

void SupervisorManager::OnMotorHeartbeatReceived() {
    heartbeat_monitor_.OnMotorHeartbeatReceived();
}

void SupervisorManager::TransmitStopCommand() {
    hce::supervisor::StopCommandMessage msg{};
    msg.reserved = 0U;
    const hce::supervisor::MessageCodec::Payload payload = hce::supervisor::MessageCodec::EncodeStopCommand(msg);
    can_bus_.Send(hce::supervisor::CanId::kStopCommand, payload.data(),
                   hce::supervisor::MessageCodec::kPayloadBytes);
}

void SupervisorManager::TransmitRunCommand() {
    hce::supervisor::RunCommandMessage msg{};
    msg.step_rate_hz = kDefaultRunStepRateHz;
    msg.direction = 0U;
    const hce::supervisor::MessageCodec::Payload payload = hce::supervisor::MessageCodec::EncodeRunCommand(msg);
    can_bus_.Send(hce::supervisor::CanId::kRunCommand, payload.data(),
                   hce::supervisor::MessageCodec::kPayloadBytes);
}

void SupervisorManager::Tick() {
    heartbeat_monitor_.OnMonitoringTick();

    AlarmInputs inputs{};
    inputs.pressure_pa = latest_pressure_pa_;
    inputs.pressure_valid = pressure_valid_;
    inputs.flow_lpm = latest_flow_lpm_;
    inputs.flow_valid = flow_valid_;
    inputs.sensor_heartbeat_faulted = heartbeat_monitor_.IsSensorNodeFaulted();
    inputs.motor_heartbeat_faulted = heartbeat_monitor_.IsMotorNodeFaulted();

    const bool faulted = alarm_logic_.Evaluate(inputs);
    overlay_.SetAlarmLed(faulted);

    const SupervisorState new_state = faulted ? SupervisorState::kStop : SupervisorState::kRun;
    if (new_state != state_ || faulted) {
        // STOP is re-broadcast every tick while faulted (fail-safe,
        // handles a Motor Node that boots or reconnects mid-fault).
        if (new_state == SupervisorState::kStop) {
            TransmitStopCommand();
        } else {
            TransmitRunCommand();
        }
    }
    state_ = new_state;
}

}  // namespace hce::supervisor_node
