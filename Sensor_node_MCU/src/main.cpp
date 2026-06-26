/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor Node Application
 *
 * Board  : STM32 NUCLEO-G474RE
 * RTOS   : Zephyr 3.7
 * Language : C++20
 */

#include "gpio_overlay.hpp"
#include "lcd_Display.hpp"
#include "sensor_manager.hpp"
#include "canfd.hpp"
#include "heartbeat.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace
{

/*
 * ============================================================
 * Application Constants
 * ============================================================
 */

constexpr k_timeout_t kSamplePeriod =
    K_SECONDS(1);

/*
 * ============================================================
 * Fatal Error Handler
 * ============================================================
 */

[[noreturn]]
void fatalError(
    const char* const message)
{
    printk("%s\r\n", message);

    while (true)
    {
        k_sleep(K_SECONDS(1));
    }
}

} // namespace

int main()
{
    /*
     * ============================================================
     * Hardware Initialization
     * ============================================================
     */

    if (!sensor_node::GpioOverlay::init())
    {
        fatalError(
            "GpioOverlay initialization failed");
    }

    /*
     * ============================================================
     * LCD Initialization
     * ============================================================
     */

    if (!sensor_node::LcdDisplay::init())
    {
        fatalError(
            "LCD initialization failed");
    }

    /*
     * ============================================================
     * CAN Initialization
     * ============================================================
     */

    if (!sensor_node::CanFd::init())
    {
        fatalError(
            "CAN initialization failed");
    }

    /*
     * ============================================================
     * Heartbeat Initialization
     * ============================================================
     */

    if (!sensor_node::Heartbeat::init())
    {
        fatalError(
            "Heartbeat initialization failed");
    }

    printk(
        "Sensor Node Started\r\n");

    /*
     * ============================================================
     * Main Loop
     * ============================================================
     */

    while (true)
    {
        /*
         * Read sensors and transmit sensor data.
         */
        sensor_node::SensorManager::update();

        /*
         * Update LCD display.
         */
        sensor_node::LcdDisplay::update();

        /*
         * Transmit heartbeat message.
         */
        sensor_node::Heartbeat::update();

        k_sleep(
            kSamplePeriod);
    }

    return 0;
}