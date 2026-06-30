#include "sensor_manager.hpp"

#include "canfd.hpp"
#include "flow_sensor.hpp"
#include "pressure_sensor.hpp"

namespace sensor_node
{

// ============================================================
// update()
// ============================================================

void SensorManager::update() noexcept
{
    // ---- READ PRESSURE SENSOR ----
    pressure_mmhg_ = PressureSensor::readPressureMmHg();

    // ---- READ FLOW SENSOR ----
    flow_ml_min_ = FlowSensor::readFlowMlMin();

    // ---- TRANSMIT SENSOR DATA VIA CAN ----
    CanFd::sendSensorData();
}

// ============================================================
// getPressureMmHg()
// ============================================================

std::uint16_t SensorManager::getPressureMmHg() noexcept
{
    return pressure_mmhg_;
}

// ============================================================
// getFlowMlMin()
// ============================================================

std::uint16_t SensorManager::getFlowMlMin() noexcept
{
    return flow_ml_min_;
}

} // namespace sensor_node