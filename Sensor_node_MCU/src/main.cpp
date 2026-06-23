#include "gpio_overlay.hpp"
#include "lcd_Display.hpp"
#include "sensor_manager.hpp"
#include "canfd.hpp"
#include "heartbeat.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace
{

constexpr k_timeout_t kSamplePeriod =
    K_SECONDS(1);

} // namespace

int main()
{
    /*
     * --------------------------------------------------------
     * GPIO / ADC / LCD Hardware
     * --------------------------------------------------------
     */
    const bool gpio_ok =
        sensor_node::GpioOverlay::init();

    if (!gpio_ok)
    {
        printk(
            "GpioOverlay initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    /*
     * --------------------------------------------------------
     * LCD
     * --------------------------------------------------------
     */
    const bool lcd_ok =
        sensor_node::LcdDisplay::init();

    if (!lcd_ok)
    {
        printk(
            "LCD initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    /*
     * --------------------------------------------------------
     * CAN-FD
     * --------------------------------------------------------
     */
    const bool can_ok =
        sensor_node::CanFd::init();

    if (!can_ok)
    {
        printk(
            "CAN initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    /*
     * --------------------------------------------------------
     * Heartbeat
     * --------------------------------------------------------
     */
    const bool heartbeat_ok =
        sensor_node::Heartbeat::init();

    if (!heartbeat_ok)
    {
        printk(
            "Heartbeat initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    printk(
        "Sensor Node Started\r\n");

    while (true)
    {
        /*
         * Read sensors
         * Send:
         *   0x300 Pressure
         *   0x301 Flow
         */
        sensor_node::SensorManager::update();

        /*
         * Update LCD
         */
        sensor_node::LcdDisplay::update();

        /*
         * Send Heartbeat
         * CAN ID 0x200
         */
        sensor_node::Heartbeat::update();

        k_sleep(
            kSamplePeriod);
    }

    return 0;
}