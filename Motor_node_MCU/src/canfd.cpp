/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * CAN-FD Protocol Layer Implementation
 * Motor Node MCU
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 */

#include "canfd.hpp"
#include "gpio_overlay.hpp"
#include "heartbeat.hpp"
#include "stepper_motor_driver.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(canfd, CONFIG_LOG_DEFAULT_LEVEL);

namespace motor_node
{

/* ============================================================
 * STATIC MEMBER
 * ============================================================
 */

const struct device *CanFd::can_dev_ = nullptr;

/* ============================================================
 * CAN INIT
 * ============================================================
 */

bool CanFd::init()
{
    can_dev_ = DEVICE_DT_GET(DT_ALIAS(canfd0));

    if (!device_is_ready(can_dev_)) {
        LOG_ERR("CAN device not ready");
        return false;
    }

    int ret = can_start(can_dev_);
    if (ret != 0) {
        LOG_ERR("CAN start failed: %d", ret);
        return false;
    }

    LOG_INF("CAN initialized successfully");

    return true;
}

/* ============================================================
 * MESSAGE PROCESSOR
 * ============================================================
 */

void CanFd::processReceivedMessage(const struct can_frame *frame)
{
    if (frame == nullptr) {
        return;
    }

    const std::uint32_t id = frame->id;

    switch (id)
    {
        case can_id::kStartMotor:
        {
            LOG_INF("CAN: START MOTOR");

            StepperMotorDriver::start();
            break;
        }

        case can_id::kStopMotor:
        {
            LOG_INF("CAN: STOP MOTOR");

            StepperMotorDriver::stop();
            break;
        }

        default:
        {
            LOG_WRN("CAN: Unknown ID 0x%X", id);
            break;
        }
    }
}

/* ============================================================
 * HEARTBEAT TX
 * ============================================================
 */

bool CanFd::sendHeartbeat()
{
    if (can_dev_ == nullptr) {
        return false;
    }

    struct can_frame frame;
    memset(&frame, 0, sizeof(frame));

    frame.id = can_id::kHeartbeatTx;
    frame.dlc = 0;

    int ret = can_send(can_dev_, &frame, K_MSEC(10), nullptr, nullptr);

    if (ret != 0) {
        LOG_ERR("CAN heartbeat send failed: %d", ret);
        return false;
    }

    return true;
}

} // namespace motor_node

/* ============================================================
 * CAN RX CALLBACK HOOK (TO BE USED FROM GPIO / ISR LAYER)
 * ============================================================
 */

namespace
{
    /**
     * @brief Internal RX handler bridge
     *
     * This function is meant to be called from Zephyr CAN ISR
     * or callback registered in main.cpp.
     */
    void can_rx_callback(const struct device *dev,
                         struct can_frame *frame,
                         void *user_data)
    {
        ARG_UNUSED(dev);
        ARG_UNUSED(user_data);

        if (frame == nullptr) {
            return;
        }

        /* Forward to protocol handler */
        motor_node::CanFd::processReceivedMessage(frame);
    }
}

/* ============================================================
 * OPTIONAL REGISTRATION FUNCTION
 * ============================================================
 */

void canfd_register_rx_callback()
{
    const struct device *dev = DEVICE_DT_GET(DT_ALIAS(canfd0));

    if (!device_is_ready(dev)) {
        return;
    }

    /* Register RX callback */
    can_add_rx_filter(dev,
                      can_rx_callback,
                      nullptr,
                      nullptr,
                      nullptr,
                      CAN_FILTER_IDE_MASK);
}

/* ============================================================
 * CLEANUP / RESET (SAFE STATE)
 * ============================================================
 */

void canfd_enter_safe_state()
{
    /* Stop motor immediately on CAN fault */
    motor_node::StepperMotorDriver::stop();
}

/* ============================================================
 * DEBUG UTILITIES
 * ============================================================
 */

#ifdef CONFIG_LOG

void canfd_debug_state()
{
    LOG_INF("CANFD Debug:");

    const struct device *dev = DEVICE_DT_GET(DT_ALIAS(canfd0));

    LOG_INF("CAN device ready: %d", device_is_ready(dev));
}

#endif

/* ============================================================
 * END OF FILE
 * ============================================================
 */