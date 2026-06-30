#include "alarm.hpp"

#include "gpio_overlay.hpp"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(alarm_module, LOG_LEVEL_INF);

/* ============================================================
 * STATIC MEMBER DEFINITIONS
 * ============================================================
 */
AlarmState Alarm::state_       = AlarmState::kArmed;
bool       Alarm::initialized_ = false;

/* ============================================================
 * INIT
 * ============================================================
 * Only called once, at boot, from main(). This is the only
 * place the latch is ever cleared, so the alarm can only be
 * reset by power-cycling / pressing the NRST button.
 * ============================================================
 */
bool Alarm::Init()
{
    state_ = AlarmState::kArmed;

    GpioOverlay::AlarmLedOff();

    initialized_ = true;

    return true;
}

/* ============================================================
 * TRIGGER (LATCH)
 * ============================================================
 */
void Alarm::Trigger()
{
    if (!initialized_)
    {
        return;
    }

    /* Already latched: no-op, latch can only clear on reset */
    if (state_ == AlarmState::kLatched)
    {
        return;
    }

    state_ = AlarmState::kLatched;

    GpioOverlay::AlarmLedOn();

    LOG_ERR("ALARM LATCHED -> PC4 ACTIVE (reset required to clear)");
}

/* ============================================================
 * STATE QUERY
 * ============================================================
 */
bool Alarm::IsLatched()
{
    return (state_ == AlarmState::kLatched);
}