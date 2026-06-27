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

#include <cstdint>
#include <zephyr/drivers/can.h>

namespace motor_node
{

/**
 * @brief CAN message identifiers for Motor Node MCU
 */
namespace can_id
{
    constexpr std::uint32_t kStartMotor = 0x400U;
    constexpr std::uint32_t kStopMotor  = 0x100U;
    constexpr std::uint32_t kHeartbeatTx = 0x201U;
}

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
    CanFd() = default;
    ~CanFd() = default;

    /**
     * @brief Initialize CAN interface
     * @return true if success
     */
    static bool init();

    /**
     * @brief Process received CAN frame
     * @param frame CAN frame pointer
     */
    static void processReceivedMessage(const struct can_frame *frame);

    /**
     * @brief Send heartbeat frame
     * @return true if transmitted
     */
    static bool sendHeartbeat();

private:
    static const struct device *can_dev_;
};

} // namespace motor_node