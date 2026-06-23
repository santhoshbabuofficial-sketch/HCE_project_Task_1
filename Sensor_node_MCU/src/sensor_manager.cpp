#include "sensor_manager.hpp"

#include "pressure_sensor.hpp"
#include "flow_sensor.hpp"
#include "canfd.hpp"

#include <cstdint>

namespace sensor_node
{

std::uint16_t SensorManager::pressure_mmhg_ = 0U;
std::uint16_t SensorManager::flow_ml_min_ = 0U;

void
SensorManager::update()
{
    /*
     * Step 1:
     * Read Pressure Sensor
     */
    pressure_mmhg_ =
        PressureSensor::readPressureMmHg();

    /*
     * Step 2:
     * Read Flow Sensor
     */
    flow_ml_min_ =
        FlowSensor::readFlowMlMin();

    /*
     * Step 3:
     * Transmit latest sensor values
     *
     * CAN ID 0x300 -> Pressure
     * CAN ID 0x301 -> Flow
     */
    CanFd::sendSensorData();
}

std::uint16_t
SensorManager::getPressureMmHg()
{
    return pressure_mmhg_;
}

std::uint16_t
SensorManager::getFlowMlMin()
{
    return flow_ml_min_;
}

} // namespace sensor_node