#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief CAN-FD communication layer.
 *
 * Handles:
 * - Heartbeat transmission
 * - Sensor data transmission
 * - Supervisor commands (E-STOP)
 */
class CanFd
{
public:

    static bool init() noexcept;

    static void sendHeartbeat() noexcept;

    static void sendSensorData() noexcept;

    static void processReceivedMessage(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length) noexcept;

private:

    static void transmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length) noexcept;

    CanFd() = delete;
};

} // namespace sensor_node