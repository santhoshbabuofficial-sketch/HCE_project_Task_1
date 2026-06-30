#pragma once

namespace sensor_node
{

/**
 * @brief System heartbeat module.
 *
 * On each update() call:
 *  1. Transmits the CAN heartbeat frame (ID 0x200, "I am Alive").
 *  2. Pulses the PC4 LED HIGH for 500 ms then LOW.
 *
 * No heap allocation.  All methods are static.
 */
class Heartbeat
{
public:

    /**
     * @brief Initialise the heartbeat module.
     *
     * Currently a no-op (hardware already configured by GpioOverlay::init()).
     * Reserved for future use (e.g., watchdog kick registration).
     *
     * @return true always.
     */
    [[nodiscard]] static bool init() noexcept;

    /**
     * @brief Transmit the CAN heartbeat frame and pulse the LED.
     *
     * Blocks the calling thread for 500 ms during the LED pulse.
     * Must be called from a thread context, not an ISR.
     */
    static void update() noexcept;

private:

    /// @cond PRIVATE
    Heartbeat() = delete;
    /// @endcond
};

} // namespace sensor_node