#ifndef ALARM_HPP
#define ALARM_HPP

/**
 * ============================================================
 * ALARM MODULE
 * ============================================================
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Latch the system into an ALARM state (PC4 LED ON)
 *   whenever a safety-critical fault condition is reported
 *   by any other module (flow/pressure out of range, missed
 *   heartbeats, E-STOP, etc.).
 * - Once latched, the alarm output (PC4) stays ACTIVE and
 *   cannot be cleared by software. The only way to clear the
 *   alarm is a full MCU reset (pressing the on-board NRST
 *   button), which re-runs static initialization and Init().
 *
 * DESIGN PATTERN: State Pattern
 * - AlarmState::kArmed  -> normal operation, PC4 OFF.
 * - AlarmState::kLatched -> fault occurred, PC4 ON, and the
 *   state machine refuses every transition back to kArmed
 *   from software. Only Init() (invoked at boot / after a
 *   hardware reset) may return the state machine to kArmed.
 *
 * RULE:
 * - No business logic (sensor evaluation, CAN decoding) here.
 * - Only alarm state management + GPIO latch behaviour.
 * ============================================================
 */

#include <cstdint>

/**
 * @brief Alarm state machine states (State design pattern).
 */
enum class AlarmState : std::uint8_t
{
    kArmed   = 0U, ///< Normal operation, alarm output inactive.
    kLatched = 1U  ///< Fault latched, alarm output active until reset.
};

/**
 * @brief Latching safety alarm driver.
 *
 * Drives the ALARM LED on PC4 through GpioOverlay. Implements a
 * latch: once Trigger() is called, the alarm remains active
 * (PC4 HIGH) regardless of how many times Trigger()/Clear() is
 * subsequently called from software. The latch can only be
 * removed by a physical MCU reset (NRST button), since Init()
 * is the sole entry point that resets the internal state, and
 * Init() is only ever invoked once at boot from main().
 */
class Alarm
{
public:
    /**
     * @brief Initialize the alarm module.
     *
     * Must be called exactly once at boot, after
     * GpioOverlay::Init(). Sets the state machine to
     * AlarmState::kArmed and ensures PC4 is OFF.
     *
     * @return true if initialization succeeded.
     */
    static bool Init();

    /**
     * @brief Trigger (latch) the alarm.
     *
     * Drives PC4 HIGH and transitions the state machine to
     * AlarmState::kLatched. Safe to call repeatedly / from
     * multiple threads (e.g. SafetyThread and
     * HeartbeatMonitoring) — once latched, further calls are
     * a no-op.
     */
    static void Trigger();

    /**
     * @brief Query whether the alarm is currently latched.
     * @return true if the alarm has been triggered and not
     *         yet cleared by a hardware reset.
     */
    static bool IsLatched();

private:
    static AlarmState state_;
    static bool       initialized_;
};

#endif // ALARM_HPP