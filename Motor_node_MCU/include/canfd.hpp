#pragma once

#include <cstdint>

namespace motor_node
{

class CanFd
{
public:
    /*
     * ==========================================================
     * Initialize CAN-FD Interface
     * ==========================================================
     */
    static bool init();

    /*
     * ==========================================================
     * CAN ID : 0x201
     * Heartbeat
     * No Payload
     * ==========================================================
     */
    static void sendHeartbeat();

    /*
     * ==========================================================
     * Process received CAN messages
     *
     * Called from:
     * GpioOverlay CAN RX Callback
     * ==========================================================
     */
    static void processReceivedMessage(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length);

private:
    static void transmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length);

private:
    CanFd() = delete;
};

} // namespace motor_node