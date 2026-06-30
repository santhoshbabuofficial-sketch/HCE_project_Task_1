#include "pressure_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

// ============================================================
// CALIBRATION CONSTANTS
// ============================================================

/// ADC count corresponding to 0 mmHg (sensor lower rail).
constexpr std::uint16_t kAdcMin = 413U;

/// ADC count corresponding to 12001 mmHg (sensor upper rail).
constexpr std::uint16_t kAdcMax = 3723U;

/// Pressure output at ADC minimum (mmHg).
constexpr std::uint16_t kPressureMinMmHg = 0U;

/// Pressure output at ADC maximum (mmHg).
constexpr std::uint16_t kPressureMaxMmHg = 12001U;

/// Total ADC span between min and max calibration points.
constexpr std::uint32_t kAdcSpan =
    static_cast<std::uint32_t>(kAdcMax - kAdcMin);

/// Total pressure span between min and max calibration points (mmHg).
constexpr std::uint32_t kPressureSpan =
    static_cast<std::uint32_t>(kPressureMaxMmHg - kPressureMinMmHg);

} // namespace

namespace sensor_node
{

// ============================================================
// readPressureMmHg()
// ============================================================

std::uint16_t PressureSensor::readPressureMmHg() noexcept
{
    const std::uint16_t adc_raw = GpioOverlay::readPressureAdcRaw();

    if (adc_raw <= kAdcMin)
    {
        return kPressureMinMmHg;
    }

    if (adc_raw >= kAdcMax)
    {
        return kPressureMaxMmHg;
    }

    const std::uint32_t offset =
        static_cast<std::uint32_t>(adc_raw - kAdcMin);

    // Linear interpolation within calibrated range
    const std::uint32_t value = (offset * kPressureSpan) / kAdcSpan;

    return static_cast<std::uint16_t>(value);
}

} // namespace sensor_node