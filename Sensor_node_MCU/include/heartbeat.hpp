#pragma once

namespace sensor_node
{

/**
 * @brief System heartbeat module.
 *
 * Sends periodic CAN alive signal.
 */
class Heartbeat
{
public:

    /**
     * @brief Initialize heartbeat system.
     */
    static bool init() noexcept;

    /**
     * @brief Send heartbeat frame.
     */
    static void update() noexcept;

private:
    Heartbeat() = delete;
};

} // namespace sensor_node