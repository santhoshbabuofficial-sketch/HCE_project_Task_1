#include "sensor_manager.hpp"

#include "pressure_sensor.hpp"
#include "flow_sensor.hpp"
#include "canfd.hpp"

#include <cstdint>

namespace sensor_node
{

void SensorManager::update() noexcept
{
    /*
     * ============================================================
     * Step 1: Read Pressure Sensor
     * ============================================================
     */
    pressure_mmhg_ =
        PressureSensor::readPressureMmHg();

    /*
     * ============================================================
     * Step 2: Read Flow Sensor
     * ============================================================
     */
    flow_ml_min_ =
        FlowSensor::readFlowMlMin();

    /*
     * ============================================================
     * Step 3: Transmit via CAN
     * (Facade triggers system-wide communication)
     * ============================================================
     */
    CanFd::sendSensorData();
}

std::uint16_t SensorManager::getPressureMmHg() noexcept
{
    return pressure_mmhg_;
}

std::uint16_t SensorManager::getFlowMlMin() noexcept
{
    return flow_ml_min_;
}

} // namespace sensor_node