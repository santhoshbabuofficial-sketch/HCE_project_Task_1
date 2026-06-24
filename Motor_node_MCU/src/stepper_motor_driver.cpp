#include "stepper_motor_driver.hpp"

#include "gpio_overlay.hpp"

namespace motor_node
{

bool StepperMotorDriver::m_is_motor_running = false;

bool
StepperMotorDriver::init()
{
    GpioOverlay::setMotorDirectionClockwise();

    GpioOverlay::disableMotor();

    GpioOverlay::stopStepPwm();

    m_is_motor_running = false;

    return true;
}

void
StepperMotorDriver::startMotor()
{
    if (m_is_motor_running)
    {
        return;
    }

    GpioOverlay::setMotorDirectionClockwise();

    GpioOverlay::enableMotor();

    if (GpioOverlay::startStepPwm())
    {
        m_is_motor_running = true;
    }
}

void
StepperMotorDriver::stopMotor()
{
    GpioOverlay::stopStepPwm();

    GpioOverlay::disableMotor();

    m_is_motor_running = false;
}

bool
StepperMotorDriver::isMotorRunning()
{
    return m_is_motor_running;
}

} // namespace motor_node