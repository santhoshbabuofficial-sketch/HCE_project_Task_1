/// @file can_bus_driver.hpp
/// @brief CAN-FD transport abstraction for the Sensor Node.
///
/// This is the ONLY translation unit permitted to call the Zephyr CAN
/// controller API directly (Rule: Hardware abstraction / Data-flow rule
/// "Application code SHALL NEVER directly access STM32 peripherals").
/// All higher layers (sensor_manager, heartbeat, lcd_display) talk to the
/// bus exclusively through gpio_overlay -> CanBusDriver.
#pragma once

#include <cstdint>

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>

namespace hce::sensor {

/// @brief Pure interface so CanBusDriver can be mocked on the host build.
class ICanBusDriver {
public:
    virtual ~ICanBusDriver() = default;

    /// @brief Bring up the CAN controller and start the bus.
    /// @return true on success.
    virtual bool Init() = 0;

    /// @brief Transmit a classic/FD CAN frame.
    /// @param can_id Standard 11-bit CAN identifier.
    /// @param data   Pointer to payload bytes.
    /// @param length Number of valid payload bytes (<= kMaxPayloadBytes).
    /// @return true if the frame was queued for transmission.
    virtual bool Send(uint32_t can_id, const uint8_t* data, uint8_t length) = 0;

    /// @brief Register a receive filter + callback for a given CAN ID.
    /// @param can_id      Identifier to filter on (exact match).
    /// @param callback    Zephyr CAN RX callback, invoked in ISR context.
    /// @param user_data   Opaque pointer forwarded to the callback.
    /// @return true if the filter was installed.
    virtual bool AddRxFilter(uint32_t can_id, can_rx_callback_t callback,
                              void* user_data) = 0;
};

/// @brief Concrete Zephyr FDCAN1 transport used by the Sensor Node.
class CanBusDriver final : public ICanBusDriver {
public:
    /// @brief Maximum payload bytes this driver will transmit/accept.
    static constexpr uint8_t kMaxPayloadBytes = 8U;

    /// @param can_dev Zephyr CAN controller device (FDCAN1 / DT_NODELABEL(fdcan1)).
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

}  // namespace hce::sensor
