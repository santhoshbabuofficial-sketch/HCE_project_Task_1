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
 * - Supervisor commands (E-Stop)
 */
class CanFd
{
public:

    /**
     * @brief Initialize CAN interface.
     */
    static bool init() noexcept;

    /**
     * @brief Send heartbeat message.
     */
    static void sendHeartbeat() noexcept;

    /**
     * @brief Send sensor data (pressure + flow).
     */
    static void sendSensorData() noexcept;

    /**
     * @brief Process received CAN message.
     */
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