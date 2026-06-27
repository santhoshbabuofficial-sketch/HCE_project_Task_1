#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Sensor aggregation and coordination layer.
 *
 * Reads all sensors and provides cached values
 * for other system modules (LCD, CAN, etc.).
 */
class SensorManager
{
public:

    /**
     * @brief Update all sensor values.
     *
     * Reads hardware sensors and triggers CAN transmission.
     */
    static void update() noexcept;

    /**
     * @brief Latest pressure value in mmHg.
     */
    static std::uint16_t getPressureMmHg() noexcept;

    /**
     * @brief Latest flow value in mL/min.
     */
    static std::uint16_t getFlowMlMin() noexcept;

private:
    SensorManager() = delete;

private:
    inline static std::uint16_t pressure_mmhg_ = 0U;
    inline static std::uint16_t flow_ml_min_   = 0U;
};

} // namespace sensor_node