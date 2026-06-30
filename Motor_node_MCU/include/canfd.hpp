/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * CAN-FD Protocol Layer (Motor Node MCU)
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Decode CAN messages
 * - Route commands to StepperMotorDriver
 * - No hardware access (only via GpioOverlay)
 */

#pragma once

#include <array>
#include <cstdint>
#include <zephyr/drivers/can.h>

namespace motor_node
{

/**
 * @brief CAN message identifiers for Motor Node MCU
 */
namespace can_id
{
    constexpr std::uint32_t kStartMotor  = 0x400U;
    constexpr std::uint32_t kStopMotor   = 0x100U;
    constexpr std::uint32_t kHeartbeatTx = 0x201U;
}

/**
 * @brief Heartbeat payload / fleet sizing constants.
 *
 * The heartbeat frame carries a single payload byte identifying
 * which node sent it, so multiple nodes can share CAN ID
 * can_id::kHeartbeatTx while still being distinguishable by
 * receivers (Rule 10: no magic numbers).
 */
namespace heartbeat_cfg
{
    /// @brief This node's unique identifier, placed in the heartbeat payload.
    constexpr std::uint8_t kThisNodeId = 0x01U;

    /// @brief Number of bytes carried in a heartbeat frame's payload.
    constexpr std::uint8_t kHeartbeatDlc = 1U;

    /// @brief Maximum number of distinct remote nodes tracked for liveness.
    constexpr std::size_t kMaxTrackedNodes = 8U;

    /// @brief Sentinel value meaning "this slot is unused".
    constexpr std::uint8_t kInvalidNodeId = 0xFFU;

    /// @brief A node is considered stale if not heard from within this window (ms).
    constexpr std::uint32_t kNodeTimeoutMs = 2000U;
}

/**
 * @brief Liveness record for a single remote node's heartbeat.
 */
struct RemoteNodeStatus
{
    /// @brief Node identifier extracted from the heartbeat payload.
    std::uint8_t nodeId = heartbeat_cfg::kInvalidNodeId;

    /// @brief Timestamp (ms, k_uptime_get_32) of the last heartbeat seen.
    std::uint32_t lastSeenMs = 0U;
};

/**
 * @brief CAN-FD protocol handler
 *
 * Responsibilities:
 * - Receive CAN frames
 * - Decode commands
 * - Call motor driver APIs
 */
class CanFd final
{
public:
    /// @brief Deleted: static-only utility class, no instantiation allowed.
    CanFd() = delete;

    /// @brief Deleted: static-only utility class.
    ~CanFd() = delete;

    /**
     * @brief Initialize CAN interface.
     * @return true if success.
     */
    static bool init();

    /**
     * @brief Process received CAN frame.
     * @param frame Pointer to CAN frame; must not be nullptr.
     */
    static void processReceivedMessage(const struct can_frame *frame);

    /**
     * @brief Send heartbeat frame.
     *
     * Transmits a 1-byte CAN-FD frame on can_id::kHeartbeatTx whose
     * payload identifies this node (heartbeat_cfg::kThisNodeId), so
     * other nodes sharing the same heartbeat ID can be told apart.
     *
     * @return true if transmitted successfully.
     */
    static bool sendHeartbeat();

    /**
     * @brief Look up the liveness record for a remote node.
     * @param nodeId Node identifier to query.
     * @return Pointer to the record if known, nullptr if never seen.
     */
    static const RemoteNodeStatus *getRemoteNodeStatus(std::uint8_t nodeId);

    /**
     * @brief Check whether a given remote node's heartbeat is still fresh.
     * @param nodeId Node identifier to query.
     * @return true if a heartbeat was seen within heartbeat_cfg::kNodeTimeoutMs.
     */
    static bool isRemoteNodeAlive(std::uint8_t nodeId);

private:
    /**
     * @brief Record/update the liveness timestamp for a remote heartbeat.
     * @param nodeId Node identifier extracted from the received payload.
     */
    static void recordRemoteHeartbeat(std::uint8_t nodeId);

private:
    static const struct device *can_dev_;

    /// @brief Fixed-size table of tracked remote nodes (Rule 1: no heap).
    static inline std::array<RemoteNodeStatus, heartbeat_cfg::kMaxTrackedNodes> remote_nodes_{};
};

} // namespace motor_node