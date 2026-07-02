#include "lcd_display.hpp"

#include <cstdio>

#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lcd_display, LOG_LEVEL_INF);

namespace hce::sensor_node {

namespace {
constexpr uint8_t kCmdClearDisplay = 0x01U;
constexpr uint8_t kCmdFunctionSet4Bit2Line = 0x28U;
constexpr uint8_t kCmdDisplayOn = 0x0CU;
constexpr uint8_t kCmdEntryModeIncrement = 0x06U;
constexpr uint8_t kRowZeroAddr = 0x80U;
constexpr uint8_t kRowOneAddr = 0xC0U;
}  // namespace

LcdDisplay::LcdDisplay(GpioOverlay& overlay) : overlay_(overlay) {}

void LcdDisplay::WriteNibble(uint8_t nibble, bool is_data) {
    uint8_t frame = static_cast<uint8_t>((nibble & 0xF0U) | kBacklightBit);
    if (is_data) {
        frame = static_cast<uint8_t>(frame | kRegisterSelectBit);
    }

    const uint8_t enable_high = static_cast<uint8_t>(frame | kEnableBit);
    i2c_write(overlay_.LcdI2cBus(), &enable_high, 1U, GpioOverlay::kLcdI2cAddr);
    k_busy_wait(1);

    const uint8_t enable_low = static_cast<uint8_t>(frame & ~kEnableBit);
    i2c_write(overlay_.LcdI2cBus(), &enable_low, 1U, GpioOverlay::kLcdI2cAddr);
    k_busy_wait(50);
}

void LcdDisplay::WriteByte(uint8_t value, bool is_data) {
    WriteNibble(static_cast<uint8_t>(value & 0xF0U), is_data);
    WriteNibble(static_cast<uint8_t>((value << 4U) & 0xF0U), is_data);
}

bool LcdDisplay::Init() {
    k_msleep(50);
    WriteNibble(0x30U, false);
    k_msleep(5);
    WriteNibble(0x30U, false);
    k_busy_wait(150);
    WriteNibble(0x30U, false);
    WriteNibble(0x20U, false);

    WriteByte(kCmdFunctionSet4Bit2Line, false);
    WriteByte(kCmdDisplayOn, false);
    WriteByte(kCmdClearDisplay, false);
    k_msleep(2);
    WriteByte(kCmdEntryModeIncrement, false);

    initialized_ = true;
    return true;
}

void LcdDisplay::SetCursor(uint8_t row, uint8_t col) {
    const uint8_t base = (row == 0U) ? kRowZeroAddr : kRowOneAddr;
    WriteByte(static_cast<uint8_t>(base + col), false);
}

void LcdDisplay::WriteLine(uint8_t row, const char* text) {
    SetCursor(row, 0U);
    for (uint8_t i = 0U; i < kLcdColumns; ++i) {
        const char c = text[i];
        if (c == '\0') {
            break;
        }
        WriteByte(static_cast<uint8_t>(c), true);
    }
}

void LcdDisplay::ShowPressure(float pressure_pa) {
    if (!initialized_) {
        return;
    }
    char line[kLcdColumns + 1U] = {0};
    std::snprintf(line, sizeof(line), "Pressure:%5d Pa", static_cast<int>(pressure_pa));
    WriteLine(0U, line);
}

void LcdDisplay::ShowFlow(float flow_lpm) {
    if (!initialized_) {
        return;
    }
    char line[kLcdColumns + 1U] = {0};
    std::snprintf(line, sizeof(line), "Flow:%7d LPM", static_cast<int>(flow_lpm));
    WriteLine(1U, line);
}

void LcdDisplay::ShowRunning() {
    WriteLine(1U, "STATUS: RUN    ");
}

void LcdDisplay::ShowStopped() {
    WriteLine(1U, "STATUS: STOP   ");
}

void LcdDisplay::ShowEmergencyStop() {
    WriteLine(0U, "E-STOP BROADCAST");
    WriteLine(1U, "                ");
}

}  // namespace hce::sensor_node
