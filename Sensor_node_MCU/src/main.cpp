#include "canfd.hpp"
#include "gpio_overlay.hpp"
#include "heartbeat.hpp"
#include "lcd_display.hpp"
#include "sensor_manager.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace
{

/// Main-loop sample period (1 second).
constexpr k_timeout_t kSamplePeriod = K_SECONDS(1);

/**
 * @brief Enter a safe fault state and spin forever.
 *
 * @param msg Null-terminated diagnostic string printed via printk.
 */
[[noreturn]] void fatalError(const char* msg)
{
    printk("%s\r\n", msg);
    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}

} // namespace

// ============================================================
// main()
// ============================================================

int main()
{
    // ---- GPIO + HAL INIT ----
    if (!sensor_node::GpioOverlay::init())
    {
        fatalError("GPIO init failed");
    }

    // ---- LCD INIT ----
    if (!sensor_node::LcdDisplay::init())
    {
        fatalError("LCD init failed");
    }

    // ---- CAN INIT ----
    if (!sensor_node::CanFd::init())
    {
        fatalError("CAN init failed");
    }

    // ---- HEARTBEAT INIT ----
    if (!sensor_node::Heartbeat::init())
    {
        fatalError("Heartbeat init failed");
    }

    printk("Sensor Node Started\r\n");

    // ============================================================
    // MAIN LOOP — 1-SECOND CYCLE
    // ============================================================
    while (true)
    {
        // Read sensors + transmit CAN data
        sensor_node::SensorManager::update();

        // Refresh LCD with latest sensor values
        sensor_node::LcdDisplay::update();

        // Send CAN heartbeat + pulse PC4 LED (500 ms)
        sensor_node::Heartbeat::update();

        k_sleep(kSamplePeriod);
    }

    return 0;
}