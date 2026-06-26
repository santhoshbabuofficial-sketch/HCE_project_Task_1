#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Pressure sensor driver (conversion layer).
 *
 * Converts raw ADC readings into calibrated pressure (mmHg).
 * Hardware access is delegated to GpioOverlay.
 */
class PressureSensor
{
public:

    /**
     * @brief Read calibrated pressure.
     *
     * @return Pressure in mmHg (0–12001).
     */
    static std::uint16_t readPressureMmHg() noexcept;

private:
    PressureSensor() = delete;
};

} // namespace sensor_node