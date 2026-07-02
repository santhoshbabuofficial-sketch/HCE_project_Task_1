#include "flow_sensor.hpp"

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(flow_sensor, LOG_LEVEL_INF);

namespace hce::sensor_node {

FlowSensor::FlowSensor(GpioOverlay& overlay) : overlay_(overlay) {}

bool FlowSensor::Init() {
    // PAV3015 requires no explicit configuration for continuous
    // free-running velocity conversion in the default power-up mode.
    initialized_ = device_is_ready(overlay_.FlowI2cBus());
    if (!initialized_) {
        LOG_ERR("Flow sensor I2C bus not ready");
    }
    return initialized_;
}

bool FlowSensor::Read(float& flow_lpm_out) {
    if (!initialized_) {
        return false;
    }

    uint8_t raw[2] = {0U, 0U};
    const int err = i2c_burst_read(overlay_.FlowI2cBus(), GpioOverlay::kFlowSensorI2cAddr,
                                    kRegVelocityMsb, raw, sizeof(raw));
    if (err != 0) {
        LOG_WRN("PAV3015 read failed: %d", err);
        return false;
    }

    const uint16_t raw_velocity =
        static_cast<uint16_t>((static_cast<uint16_t>(raw[0]) << 8) | raw[1]);

    const float velocity_mps = static_cast<float>(raw_velocity) * kVelocityLsbToMps;
    const float flow_m3_per_s = velocity_mps * kDuctAreaM2;
    flow_lpm_out = flow_m3_per_s * kM3PerSecToLitrePerMin;
    return true;
}

}  // namespace hce::sensor_node
