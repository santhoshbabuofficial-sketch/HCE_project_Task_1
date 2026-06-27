#pragma once

namespace sensor_node
{

/**
 * @brief System heartbeat module.
 *
 * Sends periodic CAN alive signal and triggers LED pulse.
 */
class Heartbeat
{
public:

    /**
     * @brief Initialize heartbeat system.
     */
    static bool init() noexcept;

    /**
     * @brief Send heartbeat frame and trigger LED.
     */
    static void update() noexcept;

private:
    Heartbeat() = delete;
};

} // namespace sensor_node