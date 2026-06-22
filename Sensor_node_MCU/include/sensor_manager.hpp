#pragma once

#include <cstdint>

namespace sensor_node
{

class SensorManager
{
public:
    /**
     * @brief Update all sensor values.
     *
     * Call periodically (e.g. every 1 second).
     */
    static void update();

    /**
     * @brief Latest pressure value.
     */
    static std::uint16_t getPressureMmHg();

    /**
     * @brief Latest flow value.
     */
    static std::uint16_t getFlowMlMin();

private:
    SensorManager() = delete;

    /*
     * Latest cached values
     */
    static std::uint16_t pressure_mmhg_;
    static std::uint16_t flow_ml_min_;
};

} // namespace sensor_node