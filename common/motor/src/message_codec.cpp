#include "message_codec.hpp"

namespace hce::motor {

namespace {
constexpr std::size_t kSequenceOffset = 0U;
constexpr std::size_t kFlagOffset = 4U;
constexpr std::size_t kStepRateOffset = 0U;
constexpr std::size_t kDirectionOffset = 4U;
}  // namespace

MessageCodec::Payload MessageCodec::EncodeHeartbeat(const HeartbeatMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kSequenceOffset], &msg.sequence, sizeof(msg.sequence));
    payload[kFlagOffset] = msg.node_ok;
    return payload;
}

HeartbeatMessage MessageCodec::DecodeHeartbeat(const Payload& payload) {
    HeartbeatMessage msg{};
    std::memcpy(&msg.sequence, &payload[kSequenceOffset], sizeof(msg.sequence));
    msg.node_ok = payload[kFlagOffset];
    return msg;
}

MessageCodec::Payload MessageCodec::EncodeStopCommand(const StopCommandMessage& msg) {
    Payload payload{};
    payload[0] = msg.reserved;
    return payload;
}

StopCommandMessage MessageCodec::DecodeStopCommand(const Payload& payload) {
    StopCommandMessage msg{};
    msg.reserved = payload[0];
    return msg;
}

MessageCodec::Payload MessageCodec::EncodeRunCommand(const RunCommandMessage& msg) {
    Payload payload{};
    std::memcpy(&payload[kStepRateOffset], &msg.step_rate_hz, sizeof(msg.step_rate_hz));
    payload[kDirectionOffset] = msg.direction;
    return payload;
}

RunCommandMessage MessageCodec::DecodeRunCommand(const Payload& payload) {
    RunCommandMessage msg{};
    std::memcpy(&msg.step_rate_hz, &payload[kStepRateOffset], sizeof(msg.step_rate_hz));
    msg.direction = payload[kDirectionOffset];
    return msg;
}

}  // namespace hce::motor
