#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief CAN-FD communication layer.
 *
 * Handles:
 * - Heartbeat transmission (ID 0x200)
 * - Sensor data transmission (IDs 0x300, 0x301)
 * - Supervisor command reception (E-STOP ID 0x101)
 *
 * All methods are static; this class cannot be instantiated.
 * No heap allocation is used.
 */
class CanFd
{
public:

    /**
     * @brief Initialise the CAN-FD controller.
     *
     * Delegates to GpioOverlay::canInit().
     *
     * @return true  Controller started and RX filter installed.
     * @return false Initialisation failed.
     */
    [[nodiscard]] static bool init() noexcept;

    /**
     * @brief Transmit the periodic heartbeat frame (ID 0x200).
     *
     * Payload: ASCII "I am Alive" (10 bytes).
     */
    static void sendHeartbeat() noexcept;

    /**
     * @brief Transmit current sensor readings via CAN-FD.
     *
     * Sends:
     * - Pressure (ID 0x300) — 2 bytes, little-endian uint16 mmHg
     * - Flow     (ID 0x301) — 2 bytes, little-endian uint16 mL/min
     */
    static void sendSensorData() noexcept;

    /**
     * @brief Dispatch a received CAN frame to the appropriate handler.
     *
     * Called from the GpioOverlay CAN RX callback (ISR context).
     *
     * @param id     Standard CAN ID of the received frame.
     * @param data   Pointer to frame payload bytes.
     * @param length Number of payload bytes (DLC).
     */
    static void processReceivedMessage(
        std::uint32_t       id,
        const std::uint8_t* data,
        std::uint8_t        length) noexcept;

private:

    /**
     * @brief Internal transmit helper.
     *
     * Validates arguments then delegates to GpioOverlay::canTransmit().
     *
     * @param id     Standard CAN ID.
     * @param data   Pointer to payload bytes. Must not be nullptr.
     * @param length Number of payload bytes. Must be > 0.
     */
    static void transmit(
        std::uint32_t       id,
        const std::uint8_t* data,
        std::uint8_t        length) noexcept;

    /// @cond PRIVATE
    CanFd() = delete;
    /// @endcond
};

} // namespace sensor_node