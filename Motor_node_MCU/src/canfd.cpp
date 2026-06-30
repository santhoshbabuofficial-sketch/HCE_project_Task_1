/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * CAN-FD Protocol Layer Implementation
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 */

#include "canfd.hpp"
#include "stepper_motor_driver.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <cstring>

LOG_MODULE_REGISTER(canfd, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * STATIC MEMBER
 * ============================================================
 */

const struct device *CanFd::can_dev_ = nullptr;

/* ============================================================
 * CAN INIT
 * ============================================================
 */

bool CanFd::init()
{
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(canfd0));

    if (!device_is_ready(can_dev_)) {
        LOG_ERR("CAN device not ready");
        return false;
    }

    int ret = can_start(can_dev_);
    if (ret != 0) {
        LOG_ERR("CAN start failed: %d", ret);
        return false;
    }

    LOG_INF("CAN initialized successfully");

    return true;
}

/* ============================================================
 * MESSAGE PROCESSOR
 * ============================================================
 */

void CanFd::processReceivedMessage(const struct can_frame *frame)
{
    if (frame == nullptr) {
        return;
    }

    const std::uint32_t id = frame->id;

    switch (id)
    {
        case can_id::kStartMotor:
        {
            LOG_INF("CAN: START MOTOR");
            StepperMotorDriver::start();
            break;
        }

        case can_id::kStopMotor:
        {
            LOG_INF("CAN: STOP MOTOR");
            StepperMotorDriver::stop();
            break;
        }

        case can_id::kHeartbeatTx:
        {
            if (frame->dlc >= heartbeat_cfg::kHeartbeatDlc) {
                const std::uint8_t remoteNodeId = frame->data[0];
                recordRemoteHeartbeat(remoteNodeId);
                LOG_INF("CAN: Heartbeat from node 0x%02X", remoteNodeId);
            } else {
                LOG_WRN("CAN: Heartbeat frame with unexpected DLC %u", frame->dlc);
            }
            break;
        }

        default:
        {
            LOG_WRN("CAN: Unknown ID 0x%X", id);
            break;
        }
    }
}

/* ============================================================
 * HEARTBEAT TX
 * ============================================================
 */

bool CanFd::sendHeartbeat()
{
    if (can_dev_ == nullptr) {
        return false;
    }

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.id      = can_id::kHeartbeatTx;
    frame.dlc     = heartbeat_cfg::kHeartbeatDlc;
    frame.data[0] = heartbeat_cfg::kThisNodeId;

    int ret = can_send(can_dev_, &frame, K_MSEC(10), nullptr, nullptr);

    if (ret != 0) {
        LOG_ERR("CAN heartbeat send failed: %d", ret);
        return false;
    }

    return true;
}

/* ============================================================
 * REMOTE NODE LIVENESS TRACKING
 * ============================================================
 * Fixed-size table (Rule 1: no heap). New nodes occupy the first
 * free (kInvalidNodeId) slot or, if the table is full and the
 * node is still unknown, the heartbeat is recorded without a
 * dedicated slot (oldest-slot eviction is left as a documented
 * future extension rather than silently growing storage).
 * ============================================================
 */

void CanFd::recordRemoteHeartbeat(std::uint8_t nodeId)
{
    const std::uint32_t now = k_uptime_get_32();

    /* Update an existing slot if this node is already tracked. */
    for (auto &slot : remote_nodes_) {
        if (slot.nodeId == nodeId) {
            slot.lastSeenMs = now;
            return;
        }
    }

    /* Otherwise claim the first free slot. */
    for (auto &slot : remote_nodes_) {
        if (slot.nodeId == heartbeat_cfg::kInvalidNodeId) {
            slot.nodeId    = nodeId;
            slot.lastSeenMs = now;
            return;
        }
    }

    LOG_WRN("CAN: Remote node table full, cannot track node 0x%02X", nodeId);
}

const RemoteNodeStatus *CanFd::getRemoteNodeStatus(std::uint8_t nodeId)
{
    for (const auto &slot : remote_nodes_) {
        if (slot.nodeId == nodeId) {
            return &slot;
        }
    }

    return nullptr;
}

bool CanFd::isRemoteNodeAlive(std::uint8_t nodeId)
{
    const RemoteNodeStatus *status = getRemoteNodeStatus(nodeId);

    if (status == nullptr) {
        return false;
    }

    const std::uint32_t now = k_uptime_get_32();

    return ((now - status->lastSeenMs) <= heartbeat_cfg::kNodeTimeoutMs);
}

} // namespace motor_node

/* ============================================================
 * SAFE STATE (called on CAN fault)
 * ============================================================
 */

void canfd_enter_safe_state()
{
    motor_node::StepperMotorDriver::stop();
}

/* ============================================================
 * DEBUG UTILITIES
 * ============================================================
 */

#ifdef CONFIG_LOG

void canfd_debug_state()
{
    LOG_INF("CANFD Debug:");

    const struct device *dev = DEVICE_DT_GET(DT_ALIAS(canfd0));

    LOG_INF("CAN device ready: %d", device_is_ready(dev));
}

#endif /* CONFIG_LOG */

/* ============================================================
 * END OF FILE
 * ============================================================
 */