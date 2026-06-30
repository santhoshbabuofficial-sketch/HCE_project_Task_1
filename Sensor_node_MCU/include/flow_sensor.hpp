#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Flow sensor driver (YF-S401).
 *
 * Converts the pulse count accumulated by the GPIO ISR into a
 * calibrated flow rate in mL/min.
 *
 * Must be called at a fixed 1-second interval to obtain accurate
 * mL/min readings (pulse count is reset on each call via
 * GpioOverlay::getAndResetFlowPulseCount()).
 *
 * No heap allocation.  All methods are static.
 */
class FlowSensor
{
public:

    /**
     * @brief Read and convert the flow rate.
     *
     * Atomically reads and clears the ISR pulse counter, then
     * applies the YF-S401 calibration factor (98 pulses / L / min).
     *
     * @return Flow rate in mL/min, saturated to 65535 on overflow.
     */
    [[nodiscard]] static std::uint16_t readFlowMlMin() noexcept;

private:

    /// @cond PRIVATE
    FlowSensor() = delete;
    /// @endcond
};

} // namespace sensor_node