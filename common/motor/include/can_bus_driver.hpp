/// @file can_bus_driver.hpp
/// @brief CAN-FD transport abstraction for the Motor Node.
/// Only this translation unit calls the Zephyr CAN controller API directly.
#pragma once

#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>

namespace hce::motor {

/// @brief Pure interface so CanBusDriver can be mocked on the host build.
class ICanBusDriver {
public:
    virtual ~ICanBusDriver() = default;
    virtual bool Init() = 0;
    virtual bool Send(uint32_t can_id, const uint8_t* data, uint8_t length) = 0;
    virtual bool AddRxFilter(uint32_t can_id, can_rx_callback_t callback,
                              void* user_data) = 0;
};

/// @brief Concrete Zephyr FDCAN1 transport used by the Motor Node.
class CanBusDriver final : public ICanBusDriver {
public:
    static constexpr uint8_t kMaxPayloadBytes = 8U;

    explicit CanBusDriver(const device* can_dev);

    bool Init() override;
    bool Send(uint32_t can_id, const uint8_t* data, uint8_t length) override;
    bool AddRxFilter(uint32_t can_id, can_rx_callback_t callback,
                      void* user_data) override;

private:
    static constexpr uint32_t kNominalBitrateBps = 1000000U;
    static constexpr uint32_t kDataPhaseBitrateBps = 4000000U;

    const device* can_dev_;
};

}  // namespace hce::motor
