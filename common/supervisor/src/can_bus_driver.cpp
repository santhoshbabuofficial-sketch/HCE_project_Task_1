#include "can_bus_driver.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(supervisor_can_bus_driver, LOG_LEVEL_INF);

namespace hce::supervisor {

CanBusDriver::CanBusDriver(const device* can_dev) : can_dev_(can_dev) {}

bool CanBusDriver::Init() {
    if (can_dev_ == nullptr || !device_is_ready(can_dev_)) {
        LOG_ERR("CAN device not ready");
        return false;
    }

    can_mode_t mode = CAN_MODE_FD;
    int err = can_set_mode(can_dev_, mode);
    if (err != 0) {
        LOG_ERR("can_set_mode failed: %d", err);
        return false;
    }

    err = can_start(can_dev_);
    if (err != 0) {
        LOG_ERR("can_start failed: %d", err);
        return false;
    }

    return true;
}

bool CanBusDriver::Send(uint32_t can_id, const uint8_t* data, uint8_t length) {
    if (can_dev_ == nullptr || data == nullptr || length > kMaxPayloadBytes) {
        return false;
    }

    can_frame frame{};
    frame.id = can_id;
    frame.dlc = can_bytes_to_dlc(length);
    frame.flags = 0U;

    for (uint8_t i = 0U; i < length; ++i) {
        frame.data[i] = data[i];
    }

    const int err = can_send(can_dev_, &frame, K_MSEC(100), nullptr, nullptr);
    if (err != 0) {
        LOG_WRN("can_send failed: %d", err);
        return false;
    }
    return true;
}

bool CanBusDriver::AddRxFilter(uint32_t can_id, can_rx_callback_t callback,
                                void* user_data) {
    if (can_dev_ == nullptr || callback == nullptr) {
        return false;
    }

    can_filter filter{};
    filter.id = can_id;
    filter.mask = CAN_STD_ID_MASK;
    filter.flags = 0U;

    const int filter_id = can_add_rx_filter(can_dev_, callback, user_data, &filter);
    if (filter_id < 0) {
        LOG_ERR("can_add_rx_filter failed: %d", filter_id);
        return false;
    }
    return true;
}

}  // namespace hce::supervisor
