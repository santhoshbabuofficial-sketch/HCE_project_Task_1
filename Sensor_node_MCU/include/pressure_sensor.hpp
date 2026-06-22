#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief Pressure sensor conversion class.
 *
 * Converts:
 *
 * ADC Count (0-4095)
 *          ↓
 * Pressure (mmHg)
 *
 * Hardware access is performed through GpioOverlay.
 */
class PressureSensor
{
public:
    /**
     * @brief Read pressure in mmHg.
     *
     * @return Pressure in mmHg.
     */
    static std::uint16_t readPressureMmHg();

private:
    PressureSensor() = delete;
};

} // namespace sensor_node