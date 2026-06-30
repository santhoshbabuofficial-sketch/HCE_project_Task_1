#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Pressure sensor driver (conversion layer).
 *
 * Reads the raw 12-bit ADC value via GpioOverlay and applies a
 * linear calibration map to produce a value in mmHg:
 *
 * ADC 413  → 0 mmHg
 * ADC 3723 → 12001 mmHg
 *
 * Values below ADC_MIN are clamped to 0 mmHg.
 * Values above ADC_MAX are clamped to 12001 mmHg.
 *
 * No heap allocation.  All methods are static.
 */
class PressureSensor
{
public:

    /**
     * @brief Read and convert the pressure sensor output.
     *
     * @return Calibrated pressure in mmHg (0–12001), clamped at extremes.
     */
    [[nodiscard]] static std::uint16_t readPressureMmHg() noexcept;

private:

    /// @cond PRIVATE
    PressureSensor() = delete;
    /// @endcond
};

} // namespace sensor_node