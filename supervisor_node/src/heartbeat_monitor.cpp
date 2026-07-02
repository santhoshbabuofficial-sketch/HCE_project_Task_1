#include "heartbeat_monitor.hpp"

namespace hce::supervisor_node {

void NodeLivenessTracker::OnHeartbeatReceived() {
    received_since_last_tick_ = true;
}

void NodeLivenessTracker::OnMonitoringTick() {
    if (received_since_last_tick_) {
        consecutive_misses_ = 0U;
    } else if (consecutive_misses_ < kMaxConsecutiveMisses) {
        ++consecutive_misses_;
    }
    received_since_last_tick_ = false;
}

void HeartbeatMonitor::OnMonitoringTick() {
    sensor_tracker_.OnMonitoringTick();
    motor_tracker_.OnMonitoringTick();
}

}  // namespace hce::supervisor_node
