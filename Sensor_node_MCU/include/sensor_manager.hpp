#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Sensor aggregation and coordination layer.
 *
 * Reads all connected sensors on each update() call and caches
 * the results for consumption by the LCD and CAN modules.
 *
 * Sensor values are updated atomically from a single thread
 * (the main loop), so no locking is required for the cached
 * members.
 *
 * No heap allocation.  All methods are static.
 */
class SensorManager
{
public:

    /**
     * @brief Read all sensors and transmit data via CAN.
     *
     * Reads PressureSensor and FlowSensor, caches the results,
     * then calls CanFd::sendSensorData().
     *
     * Must be called once per 1-second cycle from the main loop.
     */
    static void update() noexcept;

    /**
     * @brief Return the most recently sampled pressure.
     *
     * @return Pressure in mmHg (0–12001). Returns 0 before first update().
     */
    [[nodiscard]] static std::uint16_t getPressureMmHg() noexcept;

    /**
     * @brief Return the most recently sampled flow rate.
     *
     * @return Flow rate in mL/min (0–65535). Returns 0 before first update().
     */
    [[nodiscard]] static std::uint16_t getFlowMlMin() noexcept;

private:

    /// @cond PRIVATE
    SensorManager() = delete;
    /// @endcond

    inline static std::uint16_t pressure_mmhg_ = 0U;
    inline static std::uint16_t flow_ml_min_   = 0U;
};

} // namespace sensor_node