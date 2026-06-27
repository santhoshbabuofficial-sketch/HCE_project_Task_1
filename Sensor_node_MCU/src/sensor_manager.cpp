#include "sensor_manager.hpp"

#include "pressure_sensor.hpp"
#include "flow_sensor.hpp"
#include "canfd.hpp"

namespace sensor_node
{

void SensorManager::update() noexcept
{
    // ============================================================
    // READ PRESSURE SENSOR
    // ============================================================
    pressure_mmhg_ =
        PressureSensor::readPressureMmHg();

    // ============================================================
    // READ FLOW SENSOR
    // ============================================================
    flow_ml_min_ =
        FlowSensor::readFlowMlMin();

    // ============================================================
    // TRANSMIT SENSOR DATA VIA CAN
    // ============================================================
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