#include "canfd.hpp"

#include "gpio_overlay.hpp"
#include "lcd_Display.hpp"
#include "sensor_manager.hpp"

#include <cstdint>

namespace
{

// ============================================================
// CAN IDs (SYSTEM PROTOCOL)
// ============================================================

constexpr std::uint32_t kEstopCanId     = 0x101U;
constexpr std::uint32_t kHeartbeatCanId = 0x200U;
constexpr std::uint32_t kPressureCanId  = 0x300U;
constexpr std::uint32_t kFlowCanId      = 0x301U;

// Heartbeat payload = "alive"
constexpr std::uint8_t kHeartbeatData[10] =
{
    'I',' ','a','m',' ','A','l','i','v','e'
};

constexpr std::size_t kHeartbeatSize = 10U;

} // namespace

namespace sensor_node
{

bool CanFd::init() noexcept
{
    return GpioOverlay::canInit();
}

// ============================================================
// HEARTBEAT TX (0x200)
// ============================================================

void CanFd::sendHeartbeat() noexcept
{
    transmit(
        kHeartbeatCanId,
        kHeartbeatData,
        static_cast<std::uint8_t>(kHeartbeatSize));
}

// ============================================================
// SENSOR DATA TX
// ============================================================

void CanFd::sendSensorData() noexcept
{
    // ---------------- PRESSURE ----------------
    const std::uint16_t pressure =
        SensorManager::getPressureMmHg();

    const std::uint8_t p[2] =
    {
        static_cast<std::uint8_t>(pressure & 0xFFU),
        static_cast<std::uint8_t>(pressure >> 8U)
    };

    transmit(kPressureCanId, p, 2U);

    // ---------------- FLOW ----------------
    const std::uint16_t flow =
        SensorManager::getFlowMlMin();

    const std::uint8_t f[2] =
    {
        static_cast<std::uint8_t>(flow & 0xFFU),
        static_cast<std::uint8_t>(flow >> 8U)
    };

    transmit(kFlowCanId, f, 2U);
}

// ============================================================
// RX MESSAGE HANDLER
// ============================================================

void CanFd::processReceivedMessage(
    std::uint32_t id,
    const std::uint8_t* /*data*/,
    std::uint8_t length)
{
    if (length == 0U)
    {
        return;
    }

    // ============================================================
    // E-STOP HANDLER
    // ============================================================
    if (id == kEstopCanId)
    {
        LcdDisplay::showMessage("E Stop Broadcasting");
    }
}

// ============================================================
// TRANSMIT WRAPPER
// ============================================================

void CanFd::transmit(
    std::uint32_t id,
    const std::uint8_t* data,
    std::uint8_t length) noexcept
{
    if ((data == nullptr) || (length == 0U))
    {
        return;
    }

    (void)GpioOverlay::canTransmit(id, data, length);
}

} // namespace sensor_node