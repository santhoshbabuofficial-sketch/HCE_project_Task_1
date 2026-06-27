#ifndef HEARTBEAT_MONITORING_HPP
#define HEARTBEAT_MONITORING_HPP

/**
 * ============================================================
 * HEARTBEAT MONITORING MODULE
 * ============================================================
 * Board : STM32 NUCLEO-G474RE
 * RTOS  : Zephyr 3.7 LTS
 *
 * Responsibility:
 * - Track Sensor Node heartbeat (0x200)
 * - Track Motor Node heartbeat (0x201)
 * - Detect 3-cycle timeout failure
 * - Trigger E-STOP via CAN
 *
 * RULE:
 * - No CAN decoding here
 * - Only state tracking + safety decision
 * ============================================================
 */

#include <cstdint>

class HeartbeatMonitoring
{
public:
    /**
     * @brief Initialize heartbeat counters
     */
    static void Init();

    /**
     * @brief Called when sensor heartbeat is received
     */
    static void OnSensorHeartbeat();

    /**
     * @brief Called when motor heartbeat is received
     */
    static void OnMotorHeartbeat();

    /**
     * @brief Periodic evaluation (called every 1000ms)
     */
    static void Evaluate();

private:
    static constexpr std::uint8_t kMaxMissedCount = 3U;

    static std::uint8_t sensor_missed_count_;
    static std::uint8_t motor_missed_count_;

    static bool estop_triggered_;

    static void TriggerEStop();
};

#endif // HEARTBEAT_MONITORING_HPP