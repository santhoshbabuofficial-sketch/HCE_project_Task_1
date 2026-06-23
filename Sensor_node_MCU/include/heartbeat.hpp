#pragma once

namespace sensor_node
{

class Heartbeat
{
public:
    /**
     * @brief Initialize heartbeat module.
     */
    static bool init();

    /**
     * @brief Send heartbeat message.
     *
     * CAN ID : 0x200
     * Payload: "I am Alive"
     */
    static void update();

private:
    Heartbeat() = delete;
};

} // namespace sensor_node