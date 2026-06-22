#include "flow_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

/*
 * YF-S401 calibration
 *
 * Flow(L/min) = PulseCountPerSecond / 98
 *
 * Convert directly to mL/min:
 *
 * Flow(mL/min) =
 * (PulseCountPerSecond * 1000) / 98
 */
constexpr std::uint32_t kMlPerLitre = 1000U;
constexpr std::uint32_t kPulsePerLitrePerMin = 98U;

/*
 * Saturation limit
 */
constexpr std::uint16_t kMaxFlowMlMin = 65535U;

} // namespace

namespace sensor_node
{

std::uint16_t
FlowSensor::readFlowMlMin()
{
    const std::uint32_t pulse_count =
        GpioOverlay::getAndResetFlowPulseCount();

    const std::uint32_t flow_ml_min =
        (pulse_count * kMlPerLitre) /
        kPulsePerLitrePerMin;

    if (flow_ml_min > kMaxFlowMlMin)
    {
        return kMaxFlowMlMin;
    }

    return static_cast<std::uint16_t>(
        flow_ml_min);
}

} // namespace sensor_node