#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Flow sensor driver (YF-S401).
 *
 * Converts pulse count into flow rate (mL/min).
 * Must be called at fixed interval (recommended: 1 second).
 */
class FlowSensor
{
public:

    /**
     * @brief Read flow rate.
     *
     * @return Flow rate in mL/min (0–65535 saturated).
     */
    static std::uint16_t readFlowMlMin() noexcept;

private:
    FlowSensor() = delete;
};

} // namespace sensor_node