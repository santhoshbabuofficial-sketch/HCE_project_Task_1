#include "flow_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

// ============================================================
// YF-S401 Calibration Constants
// ============================================================

// Pulses per liter per minute (sensor characteristic)
constexpr std::uint32_t kPulsePerLiterPerMin = 98U;

// mL conversion factor
constexpr std::uint32_t kMilliLiterPerLiter = 1000U;

// Maximum safe output (uint16 saturation)
constexpr std::uint16_t kMaxFlowMlMin = 65535U;

} // namespace

namespace sensor_node
{

std::uint16_t FlowSensor::readFlowMlMin() noexcept
{
    const std::uint32_t pulse_count =
        GpioOverlay::getAndResetFlowPulseCount();

    // Promote to 64-bit to avoid overflow in multiplication
    const std::uint64_t scaled_pulses =
        static_cast<std::uint64_t>(pulse_count) *
        static_cast<std::uint64_t>(kMilliLiterPerLiter);

    const std::uint64_t flow_ml_min =
        scaled_pulses / kPulsePerLiterPerMin;

    if (flow_ml_min > kMaxFlowMlMin)
    {
        return kMaxFlowMlMin;
    }

    return static_cast<std::uint16_t>(flow_ml_min);
}

} // namespace sensor_node