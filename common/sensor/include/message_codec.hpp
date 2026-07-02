/// @file message_codec.hpp
/// @brief CAN payload encode/decode for all messages the Sensor Node
///        transmits or receives. No heap, no floating point in ISR
///        context — encode/decode run at thread level only.
#pragma once

#include <array>
#include <cstdint>
#include <cstring>

namespace hce::sensor {

/// @brief CAN identifiers used by the Sensor Node (see communication matrix).
namespace CanId {
inline constexpr uint32_t kPressure       = 0x300U;  ///< TX: pressure reading.
inline constexpr uint32_t kFlow           = 0x301U;  ///< TX: flow reading.
inline constexpr uint32_t kSensorHeartbeat = 0x200U; ///< TX: sensor heartbeat.
inline constexpr uint32_t kStopCommand    = 0x100U;  ///< RX: supervisor STOP.
}  // namespace CanId

/// @brief Wire layout for a pressure sample (CAN ID 0x300).
struct PressureMessage {
    float pressure_pa;    ///< Absolute pressure in pascals.
    uint32_t sequence;    ///< Monotonically increasing sample counter.
};

/// @brief Wire layout for a flow sample (CAN ID 0x301).
struct FlowMessage {
    float flow_lpm;       ///< Air flow in litres per minute.
    uint32_t sequence;    ///< Monotonically increasing sample counter.
};

/// @brief Wire layout for the sensor heartbeat (CAN ID 0x200).
struct HeartbeatMessage {
    uint32_t sequence;    ///< Monotonically increasing heartbeat counter.
    uint8_t node_ok;      ///< 1 = node healthy, 0 = degraded.
};

/// @brief Encodes/decodes Sensor Node CAN payloads.
///
/// Design pattern: this is a stateless Strategy-style codec — each message
/// type has its own Encode/Decode pair so the transport layer
/// (CanBusDriver) never needs to know about message semantics.
class MessageCodec {
public:
    static constexpr uint8_t kPayloadBytes = 8U;
    using Payload = std::array<uint8_t, kPayloadBytes>;

    /// @brief Encode a pressure sample into an 8-byte CAN-FD payload.
    static Payload EncodePressure(const PressureMessage& msg);

    /// @brief Decode a pressure sample from an 8-byte CAN-FD payload.
    static PressureMessage DecodePressure(const Payload& payload);

    /// @brief Encode a flow sample into an 8-byte CAN-FD payload.
    static Payload EncodeFlow(const FlowMessage& msg);

    /// @brief Decode a flow sample from an 8-byte CAN-FD payload.
    static FlowMessage DecodeFlow(const Payload& payload);

    /// @brief Encode a heartbeat into an 8-byte CAN-FD payload.
    static Payload EncodeHeartbeat(const HeartbeatMessage& msg);

    /// @brief Decode a heartbeat from an 8-byte CAN-FD payload.
    static HeartbeatMessage DecodeHeartbeat(const Payload& payload);
};

}  // namespace hce::sensor
