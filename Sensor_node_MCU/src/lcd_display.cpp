#include "lcd_display.hpp"

#include "gpio_overlay.hpp"
#include "sensor_manager.hpp"

#include <zephyr/sys/atomic.h>
#include <zephyr/sys/printk.h>

#include <cstdint>

namespace
{

// ============================================================
// LCD STATE FLAG
// ============================================================

/*
 * g_estop_active is written from CAN RX callback (ISR context)
 * via LcdDisplay::showMessage() and read from the main thread.
 * atomic_t provides lock-free, race-free access on ARMv7-M.
 */
atomic_t g_estop_active = ATOMIC_INIT(0);

/// 16 visible columns + null terminator.
constexpr std::uint8_t kLcdLineBufLen = 17U;

} // namespace

namespace sensor_node
{

// ============================================================
// init()
// ============================================================

bool LcdDisplay::init() noexcept
{
    return GpioOverlay::lcdInit();
}

// ============================================================
// update()
// ============================================================

void LcdDisplay::update() noexcept
{
    if (atomic_get(&g_estop_active) != 0)
    {
        return;
    }

    const std::uint16_t pressure = SensorManager::getPressureMmHg();
    const std::uint16_t flow     = SensorManager::getFlowMlMin();

    /*
     * FIX: -nostdinc++ blocks <cstdio>, <array>, and <cstdint>.
     * Use plain C arrays and cbprintf (Zephyr's snprintf equivalent)
     * which is always available in the Zephyr build environment.
     */
    char line1[kLcdLineBufLen];
    char line2[kLcdLineBufLen];

    (void)snprintk(
        line1,
        kLcdLineBufLen,
        "P:%u mmHg",
        static_cast<unsigned>(pressure));

    (void)snprintk(
        line2,
        kLcdLineBufLen,
        "F:%u mL/min",
        static_cast<unsigned>(flow));

    GpioOverlay::lcdClear();

    GpioOverlay::lcdSetCursor(0U, 0U);
    GpioOverlay::lcdPrint(line1);

    GpioOverlay::lcdSetCursor(1U, 0U);
    GpioOverlay::lcdPrint(line2);
}

// ============================================================
// showMessage()
// ============================================================

void LcdDisplay::showMessage(const char* text) noexcept
{
    if (text == nullptr)
    {
        return;
    }

    atomic_set(&g_estop_active, 1);

    GpioOverlay::lcdClear();
    GpioOverlay::lcdSetCursor(0U, 0U);
    GpioOverlay::lcdPrint(text);
}

} // namespace sensor_node