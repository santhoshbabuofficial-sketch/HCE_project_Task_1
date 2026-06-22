#pragma once

#include <cstdint>

namespace sensor_node
{

class CanFd
{
public:
    static bool init();

    /*
     * Heartbeat transmission
     * Message: "I am Alive"
     */
    static void sendHeartbeat();

    /*
     * Sensor data transmission (future use)
     * Pressure + Flow from SensorManager
     */
    static void sendSensorData();

private:
    static void transmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length);

private:
    CanFd() = delete;
};

} // namespace sensor_node