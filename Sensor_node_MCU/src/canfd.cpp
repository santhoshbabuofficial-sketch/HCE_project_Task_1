#include "canfd.hpp"

#include "gpio_overlay.hpp"
#include "lcd_display.hpp"
#include "sensor_manager.hpp"

#include <cstddef>
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

// ============================================================
// HEARTBEAT PAYLOAD — "I am Alive" (10 bytes)
// ============================================================

constexpr std::uint8_t kHeartbeatData[10U] =
{
    'I', ' ', 'a', 'm', ' ', 'A', 'l', 'i', 'v', 'e'
};

constexpr std::size_t kHeartbeatSize = sizeof(kHeartbeatData);

} // namespace

namespace sensor_node
{

// ============================================================
// init()
// ============================================================

bool CanFd::init() noexcept
{
    return GpioOverlay::canInit();
}

// ============================================================
// sendHeartbeat() — TX ID 0x200
// ============================================================

void CanFd::sendHeartbeat() noexcept
{
    transmit(
        kHeartbeatCanId,
        kHeartbeatData,
        static_cast<std::uint8_t>(kHeartbeatSize));
}

// ============================================================
// sendSensorData() — TX IDs 0x300 / 0x301
// ============================================================

void CanFd::sendSensorData() noexcept
{
    // ---- PRESSURE (little-endian uint16) ----
    const std::uint16_t pressure = SensorManager::getPressureMmHg();

    const std::uint8_t p[2U] =
    {
        static_cast<std::uint8_t>(pressure & 0xFFU),
        static_cast<std::uint8_t>(pressure >> 8U)
    };

    transmit(kPressureCanId, p, 2U);

    // ---- FLOW (little-endian uint16) ----
    const std::uint16_t flow = SensorManager::getFlowMlMin();

    const std::uint8_t f[2U] =
    {
        static_cast<std::uint8_t>(flow & 0xFFU),
        static_cast<std::uint8_t>(flow >> 8U)
    };

    transmit(kFlowCanId, f, 2U);
}

// ============================================================
// processReceivedMessage()
// ============================================================

/*
 * FIX: Original declaration had noexcept but the definition was
 * missing it — mismatched specifiers cause a -Wpedantic warning
 * (and potentially a hard error with -Werror).
 * Also: `data` is intentionally unused for the E-STOP ID; suppress
 * with a cast to void rather than leaving a named-but-unused parameter.
 */
void CanFd::processReceivedMessage(
    std::uint32_t       id,
    const std::uint8_t* data,
    std::uint8_t        length) noexcept
{
    (void)data; // payload not inspected; only the ID matters for E-STOP

    if (length == 0U)
    {
        return;
    }

    if (id == kEstopCanId)
    {
        LcdDisplay::showMessage("E Stop Broadcasting");
    }
}

// ============================================================
// transmit() — internal helper
// ============================================================

void CanFd::transmit(
    std::uint32_t       id,
    const std::uint8_t* data,
    std::uint8_t        length) noexcept
{
    if ((data == nullptr) || (length == 0U))
    {
        return;
    }

    (void)GpioOverlay::canTransmit(id, data, length);
}

} // namespace sensor_node