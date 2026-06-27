#include "heartbeat_monitoring.hpp"
#include "canfd.hpp"

#include <zephyr/kernel.h>

/* ============================================================
 * STATIC VARIABLE DEFINITIONS
 * ============================================================
 */

std::uint8_t HeartbeatMonitoring::sensor_missed_count_ = 0U;
std::uint8_t HeartbeatMonitoring::motor_missed_count_  = 0U;

bool HeartbeatMonitoring::estop_triggered_ = false;

/* ============================================================
 * INIT
 * ============================================================
 */
void HeartbeatMonitoring::Init()
{
    sensor_missed_count_ = 0U;
    motor_missed_count_  = 0U;
    estop_triggered_     = false;
}

/* ============================================================
 * CALLBACK: SENSOR HEARTBEAT RECEIVED
 * ============================================================
 */
void HeartbeatMonitoring::OnSensorHeartbeat()
{
    sensor_missed_count_ = 0U;
}

/* ============================================================
 * CALLBACK: MOTOR HEARTBEAT RECEIVED
 * ============================================================
 */
void HeartbeatMonitoring::OnMotorHeartbeat()
{
    motor_missed_count_ = 0U;
}

/* ============================================================
 * E-STOP TRIGGER
 * ============================================================
 */
void HeartbeatMonitoring::TriggerEStop()
{
    if (estop_triggered_)
    {
        return;
    }

    estop_triggered_ = true;

    CanFd::SendEStopBroadcast();
}

/* ============================================================
 * PERIODIC EVALUATION (1 second cycle)
 * ============================================================
 * RULE:
 * - If no heartbeat received → increment counter
 * - If counter >= 3 → trigger E-STOP
 * ============================================================
 */
void HeartbeatMonitoring::Evaluate()
{
    /* Sensor heartbeat timeout */
    if (sensor_missed_count_ < kMaxMissedCount)
    {
        sensor_missed_count_++;
    }

    /* Motor heartbeat timeout */
    if (motor_missed_count_ < kMaxMissedCount)
    {
        motor_missed_count_++;
    }

    /* Check failure conditions */
    if (sensor_missed_count_ >= kMaxMissedCount ||
        motor_missed_count_ >= kMaxMissedCount)
    {
        TriggerEStop();
    }
}