#include "heartbeat.hpp"
#include "canfd.hpp"
#include "gpio_overlay.hpp"

namespace sensor_node
{

bool Heartbeat::init() noexcept
{
    // No dedicated HW init required
    return true;
}

void Heartbeat::update() noexcept
{
    // ============================================================
    // STEP 1: SEND CAN HEARTBEAT (0x200)
    // ============================================================
    CanFd::sendHeartbeat();

    // ============================================================
    // STEP 2: TRIGGER PC4 LED PULSE (500ms ACTIVE HIGH)
    // ============================================================
    GpioOverlay::heartbeatLedPulse();
}

} // namespace sensor_node