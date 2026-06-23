#include "canfd.hpp"

#include "gpio_overlay.hpp"
#include "lcd_Display.hpp"
#include "sensor_manager.hpp"

#include <cstdint>

namespace
{

/*
 * ============================================================
 * CAN IDs
 * ============================================================
 */
constexpr std::uint32_t kEstopCanId     = 0x101U;
constexpr std::uint32_t kHeartbeatCanId = 0x200U;
constexpr std::uint32_t kPressureCanId  = 0x300U;
constexpr std::uint32_t kFlowCanId      = 0x301U;

/*
 * ============================================================
 * Heartbeat Payload
 * ============================================================
 */
constexpr std::uint8_t kHeartbeatData[] =
{
    'I',
    ' ',
    'a',
    'm',
    ' ',
    'A',
    'l',
    'i',
    'v',
    'e'
};

} // namespace

namespace sensor_node
{

bool
CanFd::init()
{
    return GpioOverlay::canInit();
}

void
CanFd::sendHeartbeat()
{
    transmit(
        kHeartbeatCanId,
        kHeartbeatData,
        sizeof(kHeartbeatData));
}

void
CanFd::sendSensorData()
{
    /*
     * --------------------------------------------------------
     * Pressure Message
     * CAN ID : 0x300
     * --------------------------------------------------------
     */
    const std::uint16_t pressure =
        SensorManager::getPressureMmHg();

    std::uint8_t pressure_payload[2];

    pressure_payload[0] =
        static_cast<std::uint8_t>(
            pressure & 0xFFU);

    pressure_payload[1] =
        static_cast<std::uint8_t>(
            pressure >> 8U);

    transmit(
        kPressureCanId,
        pressure_payload,
        sizeof(pressure_payload));

    /*
     * --------------------------------------------------------
     * Flow Message
     * CAN ID : 0x301
     * --------------------------------------------------------
     */
    const std::uint16_t flow =
        SensorManager::getFlowMlMin();

    std::uint8_t flow_payload[2];

    flow_payload[0] =
        static_cast<std::uint8_t>(
            flow & 0xFFU);

    flow_payload[1] =
        static_cast<std::uint8_t>(
            flow >> 8U);

    transmit(
        kFlowCanId,
        flow_payload,
        sizeof(flow_payload));
}

void
CanFd::processReceivedMessage(
    const std::uint32_t id,
    const std::uint8_t*,
    const std::uint8_t)
{
    /*
     * --------------------------------------------------------
     * Supervisor -> Sensor Node
     * 0x101
     * --------------------------------------------------------
     */
    if (id == kEstopCanId)
    {
        LcdDisplay::showMessage(
            "E Stop Broadcasting");
    }
}

void
CanFd::transmit(
    const std::uint32_t id,
    const std::uint8_t* data,
    const std::uint8_t length)
{
    (void)GpioOverlay::canTransmit(
        id,
        data,
        length);
}

} // namespace sensor_node