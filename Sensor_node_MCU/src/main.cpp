#include "gpio_overlay.hpp"
#include "lcd_Display.hpp"
#include "sensor_manager.hpp"
#include "canfd.hpp"
#include "heartbeat.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace
{

constexpr k_timeout_t kSamplePeriod = K_SECONDS(1);

[[noreturn]]
void fatalError(const char* msg)
{
    printk("%s\r\n", msg);

    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}

} // namespace

int main()
{
    // ============================================================
    // GPIO + HAL INIT
    // ============================================================

    if (!sensor_node::GpioOverlay::init())
    {
        fatalError("GPIO init failed");
    }

    // ============================================================
    // LCD INIT
    // ============================================================

    if (!sensor_node::LcdDisplay::init())
    {
        fatalError("LCD init failed");
    }

    // ============================================================
    // CAN INIT
    // ============================================================

    if (!sensor_node::CanFd::init())
    {
        fatalError("CAN init failed");
    }

    // ============================================================
    // HEARTBEAT INIT
    // ============================================================

    if (!sensor_node::Heartbeat::init())
    {
        fatalError("Heartbeat init failed");
    }

    printk("Sensor Node Started\r\n");

    // ============================================================
    // MAIN LOOP (1 SECOND CYCLE)
    // ============================================================

    while (true)
    {
        // SENSOR + CAN TX
        sensor_node::SensorManager::update();

        // LCD UPDATE
        sensor_node::LcdDisplay::update();

        // HEARTBEAT + LED PULSE
        sensor_node::Heartbeat::update();

        k_sleep(kSamplePeriod);
    }

    return 0;
}