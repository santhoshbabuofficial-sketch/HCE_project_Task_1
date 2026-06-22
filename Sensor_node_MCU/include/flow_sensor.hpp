#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Flow sensor conversion class.
 *
 * Converts:
 *
 * Pulse Count
 *      ↓
 * Flow Rate (mL/min)
 *
 * Uses YF-S401 calibration.
 */
class FlowSensor
{
public:
    /**
     * @brief Read flow rate in mL/min.
     *
     * Expected usage:
     * - Call once every 1 second
     * - Pulse counter is reset after reading
     *
     * @return Flow rate in mL/min.
     */
    static std::uint16_t readFlowMlMin();

private:
    FlowSensor() = delete;
};

} // namespace sensor_node