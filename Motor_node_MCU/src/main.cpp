#include "canfd.hpp"
#include "gpio_overlay.hpp"
#include "heartbeat.hpp"
#include "stepper_motor_driver.hpp"

#include <zephyr/kernel.h>

namespace
{

constexpr std::int32_t kHeartbeatPeriodMs = 1000;

}

int main()
{
    if (!motor_node::GpioOverlay::init())
    {
        return -1;
    }

    if (!motor_node::CanFd::init())
    {
        return -1;
    }

    if (!motor_node::StepperMotorDriver::init())
    {
        return -1;
    }

    if (!motor_node::Heartbeat::init())
    {
        return -1;
    }

    while (true)
    {
        motor_node::Heartbeat::update();

        k_sleep(
            K_MSEC(
                kHeartbeatPeriodMs));
    }

    return 0;
}