#include "pressure_sensor.hpp"

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pressure_sensor, LOG_LEVEL_INF);

namespace hce::sensor_node {

PressureSensor::PressureSensor(GpioOverlay& overlay) : overlay_(overlay) {}

bool PressureSensor::Init() {
    uint8_t who_am_i = 0U;
    int err = i2c_reg_read_byte(overlay_.PressureI2cBus(), GpioOverlay::kPressureSensorI2cAddr,
                                 kRegWhoAmI, &who_am_i);
    if (err != 0 || who_am_i != kWhoAmIValue) {
        LOG_ERR("LPS22HB WHO_AM_I mismatch (err=%d, val=0x%02x)", err, who_am_i);
        return false;
    }

    err = i2c_reg_write_byte(overlay_.PressureI2cBus(), GpioOverlay::kPressureSensorI2cAddr,
                              kRegCtrl1, kCtrl1OdrOneHz);
    if (err != 0) {
        LOG_ERR("Failed to configure LPS22HB CTRL1: %d", err);
        return false;
    }

    initialized_ = true;
    return true;
}

bool PressureSensor::Read(float& pressure_pa_out) {
    if (!initialized_) {
        return false;
    }

    uint8_t raw[3] = {0U, 0U, 0U};
    const int err = i2c_burst_read(overlay_.PressureI2cBus(), GpioOverlay::kPressureSensorI2cAddr,
                                    static_cast<uint8_t>(kRegPressOutXl | kAutoIncrementBit), raw,
                                    sizeof(raw));
    if (err != 0) {
        LOG_WRN("LPS22HB read failed: %d", err);
        return false;
    }

    const int32_t raw_pressure =
        (static_cast<int32_t>(raw[2]) << 16) | (static_cast<int32_t>(raw[1]) << 8) | raw[0];

    const float pressure_hpa = static_cast<float>(raw_pressure) * kPressureLsbToHpa;
    pressure_pa_out = pressure_hpa * kHpaToPa;
    return true;
}

}  // namespace hce::sensor_node
