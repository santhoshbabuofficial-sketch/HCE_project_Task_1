#pragma once

#include <cstdint>

namespace sensor_node
{

class CanFd
{
public:
    /**
     * Initialize CAN-FD interface.
     */
    static bool init();

    /**
     * CAN ID: 0x200
     * Payload: "I am Alive"
     */
    static void sendHeartbeat();

    /**
     * CAN ID: 0x300
     * Pressure Sensor Data
     *
     * CAN ID: 0x301
     * Flow Sensor Data
     */
    static void sendSensorData();

    /**
     * Process received CAN messages.
     *
     * Called from:
     * GpioOverlay CAN RX callback.
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

} // namespace sensor_node