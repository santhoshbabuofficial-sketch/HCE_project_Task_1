#include "sensor_manager.hpp"

#include "pressure_sensor.hpp"
#include "flow_sensor.hpp"

/*
 * CAN-FD integration
 */
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
     * Step 1: Read sensors
     */
    pressure_mmhg_ =
        PressureSensor::readPressureMmHg();

    flow_ml_min_ =
        FlowSensor::readFlowMlMin();

    /*
     * Step 2: Send via CAN-FD
     *
     * This keeps system real-time synced:
     * sensor update → CAN transmit
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