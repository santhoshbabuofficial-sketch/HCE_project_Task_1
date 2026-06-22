#include "canfd.hpp"

#include "sensor_manager.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/kernel.h>

namespace sensor_node
{

namespace
{
/*
 * FDCAN1 device
 */
const device* g_fdcan_dev =
    DEVICE_DT_GET(DT_NODELABEL(fdcan1));

/*
 * Heartbeat CAN ID
 */
constexpr std::uint32_t kHeartbeatCanId = 0x200U;

/*
 * Heartbeat payload: "I am Alive"
 */
constexpr std::uint8_t kHeartbeatData[10] =
{
    'I',' ','a','m',' ','A','l','i','v','e'
};

/*
 * CAN timing (simple default config placeholder)
 * You may tune later in Kconfig if needed
 */
can_timing_config g_timing_cfg {};
can_filter g_filter {};
}

bool
CanFd::init()
{
    if (!device_is_ready(g_fdcan_dev))
    {
        return false;
    }

    /*
     * Basic CAN start (no filter strictness yet)
     */
    g_filter.id = 0U;
    g_filter.mask = 0U;
    g_filter.flags = CAN_FILTER_DATA | CAN_FILTER_REMOTE;

    (void)can_add_rx_filter(
        g_fdcan_dev,
        nullptr,
        &g_filter);

    (void)can_start(g_fdcan_dev);

    return true;
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
    const std::uint16_t pressure =
        SensorManager::getPressureMmHg();

    const std::uint16_t flow =
        SensorManager::getFlowMlMin();

    std::uint8_t payload[4] {};

    payload[0] = static_cast<std::uint8_t>(pressure & 0xFFU);
    payload[1] = static_cast<std::uint8_t>(pressure >> 8U);

    payload[2] = static_cast<std::uint8_t>(flow & 0xFFU);
    payload[3] = static_cast<std::uint8_t>(flow >> 8U);

    transmit(
        0x201U,
        payload,
        4U);
}

void
CanFd::transmit(
    std::uint32_t id,
    const std::uint8_t* data,
    std::uint8_t length)
{
    can_frame frame {};

    frame.id = id;
    frame.dlc = length;
    frame.flags = 0U;

    for (std::uint8_t i = 0U; i < length; ++i)
    {
        frame.data[i] = data[i];
    }

    (void)can_send(
        g_fdcan_dev,
        &frame,
        K_MSEC(10),
        nullptr,
        nullptr);
}

} // namespace sensor_node