#include "pressure_sensor.hpp"
#include "gpio_overlay.hpp"

#include <cstdint>

namespace
{

constexpr std::uint16_t kAdcMin = 413U;
constexpr std::uint16_t kAdcMax = 3723U;

constexpr std::uint16_t kPressureMinMmHg = 0U;
constexpr std::uint16_t kPressureMaxMmHg = 12001U;

constexpr std::uint32_t kAdcSpan =
    static_cast<std::uint32_t>(kAdcMax - kAdcMin);

constexpr std::uint32_t kPressureSpan =
    static_cast<std::uint32_t>(kPressureMaxMmHg - kPressureMinMmHg);

} // namespace

namespace sensor_node
{

std::uint16_t PressureSensor::readPressureMmHg() noexcept
{
    const std::uint16_t adc_raw =
        GpioOverlay::readPressureAdcRaw();

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

    const std::uint32_t value =
        (offset * kPressureSpan) / kAdcSpan;

    return static_cast<std::uint16_t>(value);
}

} // namespace sensor_node