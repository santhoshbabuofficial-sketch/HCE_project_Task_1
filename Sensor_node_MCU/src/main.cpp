#include "gpio_overlay.hpp"
#include "sensor_manager.hpp"
#include "lcd_display.hpp"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

namespace
{

constexpr k_timeout_t kSamplePeriod =
    K_SECONDS(1);

} // namespace

int main()
{
    const bool gpio_ok =
        sensor_node::GpioOverlay::init();

    if (!gpio_ok)
    {
        printk("GPIO initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    const bool lcd_ok =
        sensor_node::LcdDisplay::init();

    if (!lcd_ok)
    {
        printk("LCD initialization failed\r\n");

        while (true)
        {
            k_sleep(K_SECONDS(1));
        }
    }

    printk("Sensor node started\r\n");

    while (true)
    {
        sensor_node::SensorManager::update();

        const std::uint16_t pressure_mmhg =
            sensor_node::SensorManager::getPressureMmHg();

        const std::uint16_t flow_ml_min =
            sensor_node::SensorManager::getFlowMlMin();

        printk(
            "Pressure: %u mmHg | Flow: %u mL/min\r\n",
            pressure_mmhg,
            flow_ml_min);

        sensor_node::LcdDisplay::update();

        k_sleep(kSamplePeriod);
    }

    return 0;
}