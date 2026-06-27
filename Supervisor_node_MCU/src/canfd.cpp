#include "canfd.hpp"

#include "gpio_overlay.hpp"
#include "heartbeat_monitoring.hpp"

#include <zephyr/kernel.h>
#include <zephyr/drivers/can.h>
#include <zephyr/device.h>

/* ============================================================
 * EXTERNAL STATE (from main.cpp)
 * ============================================================
 */
extern volatile std::uint16_t g_flow_value;
extern volatile std::uint16_t g_pressure_value;

/* ============================================================
 * CAN DEVICE
 * ============================================================
 */
namespace
{
    const struct device* can_dev = DEVICE_DT_GET(DT_ALIAS(canbus0));
}

/* ============================================================
 * INIT
 * ============================================================
 */
bool CanFd::Init()
{
    if (!device_is_ready(can_dev))
    {
        return false;
    }

    if (can_start(can_dev) != 0)
    {
        return false;
    }

    return true;
}

/* ============================================================
 * RX LOOP
 * ============================================================
 */
void CanFd::ProcessRx()
{
    struct can_frame frame;

    while (true)
    {
        int ret = can_receive(can_dev, &frame, K_MSEC(10));

        if (ret == 0)
        {
            DecodeRx(frame.id, frame.data, frame.dlc);
        }

        k_sleep(K_MSEC(1));
    }
}

/* ============================================================
 * RX DECODER (REAL DATA INTEGRATION)
 * ============================================================
 */
void CanFd::DecodeRx(std::uint32_t id,
                     const std::uint8_t* data,
                     std::uint8_t len)
{
    (void)len;

    switch (id)
    {
        /* ====================================================
         * SENSOR HEARTBEAT
         * ====================================================
         */
        case kSensorHbId:
        {
            HeartbeatMonitoring::OnSensorHeartbeat();
            break;
        }

        /* ====================================================
         * MOTOR HEARTBEAT
         * ====================================================
         */
        case kMotorHbId:
        {
            HeartbeatMonitoring::OnMotorHeartbeat();
            break;
        }

        /* ====================================================
         * PRESSURE SENSOR (0x300)
         * ====================================================
         * Format: uint16_t (mmHg)
         */
        case kPressureId:
        {
            std::uint16_t value =
                static_cast<std::uint16_t>(data[0]) |
                (static_cast<std::uint16_t>(data[1]) << 8);

            g_pressure_value = value;
            break;
        }

        /* ====================================================
         * FLOW SENSOR (0x301)
         * ====================================================
         * Format: uint16_t (mL/min)
         */
        case kFlowId:
        {
            std::uint16_t value =
                static_cast<std::uint16_t>(data[0]) |
                (static_cast<std::uint16_t>(data[1]) << 8);

            g_flow_value = value;
            break;
        }

        default:
        {
            /* Unknown frame ignored */
            break;
        }
    }
}

/* ============================================================
 * TX FUNCTIONS (UNCHANGED)
 * ============================================================
 */

void CanFd::SendStartMotor()
{
    struct can_frame frame = {};

    frame.id = kStartMotorId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(10));
}

void CanFd::SendStopMotor()
{
    struct can_frame frame = {};

    frame.id = kStopMotorId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(10));
}

void CanFd::SendEStopBroadcast()
{
    struct can_frame frame = {};

    frame.id = kEstopId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(10));
}