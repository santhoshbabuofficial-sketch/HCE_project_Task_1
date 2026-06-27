#include "flow_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

// ============================================================
// SENSOR CALIBRATION CONSTANTS (YF-S401)
// ============================================================

// pulses per liter per minute
constexpr std::uint32_t kPulsePerLiterPerMin = 98U;

// conversion factor
constexpr std::uint32_t kMilliLiterPerLiter = 1000U;

// safety saturation limit
constexpr std::uint16_t kMaxFlowMlMin = 65535U;

} // namespace

namespace sensor_node
{

std::uint16_t FlowSensor::readFlowMlMin() noexcept
{
    const std::uint32_t pulse_count =
        GpioOverlay::getAndResetFlowPulseCount();

    // prevent overflow in multiplication
    const std::uint64_t scaled =
        static_cast<std::uint64_t>(pulse_count) *
        static_cast<std::uint64_t>(kMilliLiterPerLiter);

    const std::uint64_t flow =
        scaled / kPulsePerLiterPerMin;

    if (flow > kMaxFlowMlMin)
    {
        return kMaxFlowMlMin;
    }

    return static_cast<std::uint16_t>(flow);
}

} // namespace sensor_node