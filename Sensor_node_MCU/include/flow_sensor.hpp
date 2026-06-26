#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Flow sensor driver (YF-S401).
 *
 * Converts pulse count into flow rate (mL/min).
 * Must be called at fixed interval (1 second recommended).
 */
class FlowSensor
{
public:

    /**
     * @brief Compute flow rate.
     *
     * @return Flow rate in mL/min.
     */
    static std::uint16_t readFlowMlMin() noexcept;

private:
    FlowSensor() = delete;
};

} // namespace sensor_node