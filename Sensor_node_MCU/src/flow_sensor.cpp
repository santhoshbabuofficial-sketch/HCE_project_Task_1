#include "flow_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

// ============================================================
// SENSOR CALIBRATION CONSTANTS (YF-S401)
// ============================================================

/// Pulses per litre per minute (factory calibration value).
constexpr std::uint32_t kPulsePerLiterPerMin = 98U;

/// Conversion factor: litres → millilitres.
constexpr std::uint32_t kMilliLiterPerLiter = 1000U;

/// Maximum representable flow in uint16_t (saturation guard).
constexpr std::uint16_t kMaxFlowMlMin = 65535U;

} // namespace

namespace sensor_node
{

// ============================================================
// readFlowMlMin()
// ============================================================

std::uint16_t FlowSensor::readFlowMlMin() noexcept
{
    const std::uint32_t pulse_count =
        GpioOverlay::getAndResetFlowPulseCount();

    /*
     * Widening to uint64 prevents overflow in the multiplication before
     * the division reduces the magnitude back to uint16 range.
     *
     * formula: flow_mL_per_min = (pulses * 1000) / 98
     */
    const std::uint64_t scaled =
        static_cast<std::uint64_t>(pulse_count) *
        static_cast<std::uint64_t>(kMilliLiterPerLiter);

    const std::uint64_t flow = scaled / kPulsePerLiterPerMin;

    if (flow > static_cast<std::uint64_t>(kMaxFlowMlMin))
    {
        return kMaxFlowMlMin;
    }

    return static_cast<std::uint16_t>(flow);
}

} // namespace sensor_node