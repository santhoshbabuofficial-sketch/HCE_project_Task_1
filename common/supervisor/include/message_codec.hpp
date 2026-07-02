/// @file message_codec.hpp
/// @brief CAN payload encode/decode for all messages the Supervisor Node
///        transmits or receives.
#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace hce::supervisor {

/// @brief CAN identifiers used by the Supervisor Node.
namespace CanId {
inline constexpr uint32_t kPressure        = 0x300U; ///< RX: pressure reading.
inline constexpr uint32_t kFlow            = 0x301U; ///< RX: flow reading.
inline constexpr uint32_t kSensorHeartbeat = 0x200U; ///< RX: sensor heartbeat.
inline constexpr uint32_t kMotorHeartbeat  = 0x201U; ///< RX: motor heartbeat.
inline constexpr uint32_t kStopCommand     = 0x100U; ///< TX: STOP broadcast.
inline constexpr uint32_t kRunCommand      = 0x400U; ///< TX: RUN command.
}  // namespace CanId

/// @brief Wire layout for a pressure sample (CAN ID 0x300, RX).
struct PressureMessage {
    float pressure_pa;
    uint32_t sequence;
};

/// @brief Wire layout for a flow sample (CAN ID 0x301, RX).
struct FlowMessage {
    float flow_lpm;
    uint32_t sequence;
};

/// @brief Wire layout for a heartbeat (CAN ID 0x200 or 0x201, RX).
struct HeartbeatMessage {
    uint32_t sequence;
    uint8_t node_ok;
};

/// @brief Wire layout for the STOP command (CAN ID 0x100, TX).
struct StopCommandMessage {
    uint8_t reserved;
};

/// @brief Wire layout for the RUN command (CAN ID 0x400, TX).
struct RunCommandMessage {
    uint32_t step_rate_hz;
    uint8_t direction;
};

/// @brief Encodes/decodes Supervisor Node CAN payloads (stateless codec).
class MessageCodec {
public:
    static constexpr uint8_t kPayloadBytes = 8U;
    using Payload = std::array<uint8_t, kPayloadBytes>;

    static PressureMessage DecodePressure(const Payload& payload);
    static FlowMessage DecodeFlow(const Payload& payload);
    static HeartbeatMessage DecodeHeartbeat(const Payload& payload);

    static Payload EncodeStopCommand(const StopCommandMessage& msg);
    static Payload EncodeRunCommand(const RunCommandMessage& msg);
};

}  // namespace hce::supervisor
