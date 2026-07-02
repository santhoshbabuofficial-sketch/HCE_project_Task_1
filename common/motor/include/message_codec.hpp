/// @file message_codec.hpp
/// @brief CAN payload encode/decode for all messages the Motor Node
///        transmits or receives.
#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace hce::motor {

/// @brief CAN identifiers used by the Motor Node.
namespace CanId {
inline constexpr uint32_t kMotorHeartbeat = 0x201U; ///< TX: motor heartbeat.
inline constexpr uint32_t kStopCommand    = 0x100U; ///< RX: supervisor STOP.
inline constexpr uint32_t kRunCommand     = 0x400U; ///< RX: supervisor RUN.
}  // namespace CanId

/// @brief Wire layout for the motor heartbeat (CAN ID 0x201).
struct HeartbeatMessage {
    uint32_t sequence;  ///< Monotonically increasing heartbeat counter.
    uint8_t node_ok;    ///< 1 = node healthy, 0 = degraded.
};

/// @brief Wire layout for the STOP command (CAN ID 0x100). Empty body;
/// the identifier alone conveys the command.
struct StopCommandMessage {
    uint8_t reserved;
};

/// @brief Wire layout for the RUN command (CAN ID 0x400).
struct RunCommandMessage {
    uint32_t step_rate_hz;  ///< Requested STEP pulse frequency.
    uint8_t direction;      ///< 0 = forward, 1 = reverse.
};

/// @brief Encodes/decodes Motor Node CAN payloads (stateless Strategy codec).
class MessageCodec {
public:
    static constexpr uint8_t kPayloadBytes = 8U;
    using Payload = std::array<uint8_t, kPayloadBytes>;

    static Payload EncodeHeartbeat(const HeartbeatMessage& msg);
    static HeartbeatMessage DecodeHeartbeat(const Payload& payload);

    static Payload EncodeStopCommand(const StopCommandMessage& msg);
    static StopCommandMessage DecodeStopCommand(const Payload& payload);

    static Payload EncodeRunCommand(const RunCommandMessage& msg);
    static RunCommandMessage DecodeRunCommand(const Payload& payload);
};

}  // namespace hce::motor
