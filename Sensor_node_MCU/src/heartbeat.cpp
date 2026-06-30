#include "heartbeat.hpp"

#include "canfd.hpp"
#include "gpio_overlay.hpp"

namespace sensor_node
{

// ============================================================
// init()
// ============================================================

bool Heartbeat::init() noexcept
{
    // No dedicated hardware init required; GpioOverlay::init()
    // already configured PC4.
    return true;
}

// ============================================================
// update()
// ============================================================

void Heartbeat::update() noexcept
{
    // ---- STEP 1: SEND CAN HEARTBEAT (ID 0x200) ----
    CanFd::sendHeartbeat();

    // ---- STEP 2: PULSE PC4 LED (500 ms ACTIVE HIGH) ----
    GpioOverlay::heartbeatLedPulse();
}

} // namespace sensor_node