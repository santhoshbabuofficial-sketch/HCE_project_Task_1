#include "message_codec.hpp"

namespace hce::sensor {

namespace {
constexpr std::size_t kFloatOffset = 0U;
constexpr std::size_t kSequenceOffset = 4U;
constexpr std::size_t kFlagOffset = 4U;
}  // namespace

MessageCodec::Payload MessageCodec::EncodePressure(const PressureMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kFloatOffset], &msg.pressure_pa, sizeof(msg.pressure_pa));
    std::memcpy(&payload[kSequenceOffset], &msg.sequence, sizeof(msg.sequence));
    return payload;
}

PressureMessage MessageCodec::DecodePressure(const Payload& payload) {
    PressureMessage msg{};
    std::memcpy(&msg.pressure_pa, &payload[kFloatOffset], sizeof(msg.pressure_pa));
    std::memcpy(&msg.sequence, &payload[kSequenceOffset], sizeof(msg.sequence));
    return msg;
}

MessageCodec::Payload MessageCodec::EncodeFlow(const FlowMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kFloatOffset], &msg.flow_lpm, sizeof(msg.flow_lpm));
    std::memcpy(&payload[kSequenceOffset], &msg.sequence, sizeof(msg.sequence));
    return payload;
}

FlowMessage MessageCodec::DecodeFlow(const Payload& payload) {
    FlowMessage msg{};
    std::memcpy(&msg.flow_lpm, &payload[kFloatOffset], sizeof(msg.flow_lpm));
    std::memcpy(&msg.sequence, &payload[kSequenceOffset], sizeof(msg.sequence));
    return msg;
}

MessageCodec::Payload MessageCodec::EncodeHeartbeat(const HeartbeatMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kFloatOffset], &msg.sequence, sizeof(msg.sequence));
    payload[kFlagOffset] = msg.node_ok;
    return payload;
}

HeartbeatMessage MessageCodec::DecodeHeartbeat(const Payload& payload) {
    HeartbeatMessage msg{};
    std::memcpy(&msg.sequence, &payload[kFloatOffset], sizeof(msg.sequence));
    msg.node_ok = payload[kFlagOffset];
    return msg;
}

}  // namespace hce::sensor
