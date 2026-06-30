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

    /* Zephyr's CAN driver API is filter + callback based; there is no
     * blocking can_receive(). can_add_rx_filter_msgq() registers an
     * internal ISR-safe callback that copies matching frames straight
     * into this fixed-size queue, which ProcessRx() then drains in
     * thread context. No heap allocation: storage is static. */
    K_MSGQ_DEFINE(can_rx_msgq,
                  sizeof(struct can_frame),
                  CanFd::kRxQueueDepth,
                  alignof(struct can_frame));
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

    /* Route every received standard-ID frame into can_rx_msgq via
     * Zephyr's built-in ISR-safe filter callback. */
    struct can_filter filter = {};
    filter.id   = kAcceptAllId;
    filter.mask = kAcceptAllMask;

    const int filter_id = can_add_rx_filter_msgq(can_dev, &can_rx_msgq, &filter);
    if (filter_id < 0)
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
        const int ret = k_msgq_get(&can_rx_msgq, &frame, K_MSEC(kRxPollTimeoutMs));

        if (ret == 0)
        {
            DecodeRx(frame.id, frame.data, frame.dlc);
        }
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
 * TX FUNCTIONS
 * ============================================================
 */

void CanFd::SendStartMotor()
{
    struct can_frame frame = {};

    frame.id = kStartMotorId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(kTxTimeoutMs), nullptr, nullptr);
}

void CanFd::SendStopMotor()
{
    struct can_frame frame = {};

    frame.id = kStopMotorId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(kTxTimeoutMs), nullptr, nullptr);
}

void CanFd::SendEStopBroadcast()
{
    struct can_frame frame = {};

    frame.id = kEstopId;
    frame.dlc = 0U;

    (void)can_send(can_dev, &frame, K_MSEC(kTxTimeoutMs), nullptr, nullptr);
}