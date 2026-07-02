#include "message_codec.hpp"

namespace hce::supervisor {

namespace {
constexpr std::size_t kFloatOffset = 0U;
constexpr std::size_t kSequenceOffset = 4U;
constexpr std::size_t kHeartbeatSeqOffset = 0U;
constexpr std::size_t kHeartbeatFlagOffset = 4U;
constexpr std::size_t kStepRateOffset = 0U;
constexpr std::size_t kDirectionOffset = 4U;
}  // namespace

PressureMessage MessageCodec::DecodePressure(const Payload& payload) {
    PressureMessage msg{};
    std::memcpy(&msg.pressure_pa, &payload[kFloatOffset], sizeof(msg.pressure_pa));
    std::memcpy(&msg.sequence, &payload[kSequenceOffset], sizeof(msg.sequence));
    return msg;
}

FlowMessage MessageCodec::DecodeFlow(const Payload& payload) {
    FlowMessage msg{};
    std::memcpy(&msg.flow_lpm, &payload[kFloatOffset], sizeof(msg.flow_lpm));
    std::memcpy(&msg.sequence, &payload[kSequenceOffset], sizeof(msg.sequence));
    return msg;
}

HeartbeatMessage MessageCodec::DecodeHeartbeat(const Payload& payload) {
    HeartbeatMessage msg{};
    std::memcpy(&msg.sequence, &payload[kHeartbeatSeqOffset], sizeof(msg.sequence));
    msg.node_ok = payload[kHeartbeatFlagOffset];
    return msg;
}

MessageCodec::Payload MessageCodec::EncodeStopCommand(const StopCommandMessage& msg) {
    Payload payload{};
    payload[0] = msg.reserved;
    return payload;
}

MessageCodec::Payload MessageCodec::EncodeRunCommand(const RunCommandMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kStepRateOffset], &msg.step_rate_hz, sizeof(msg.step_rate_hz));
    payload[kDirectionOffset] = msg.direction;
    return payload;
}

}  // namespace hce::supervisor
