#include "gpio_overlay.hpp"

#include "canfd.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>

namespace
{

/*
 * ============================================================
 * Motor Driver GPIO
 * ============================================================
 */

/*
 * PB0 -> DIR
 */
const gpio_dt_spec g_dir_gpio =
    GPIO_DT_SPEC_GET(
        DT_ALIAS(motor_dir),
        gpios);

/*
 * PB1 -> EN
 */
const gpio_dt_spec g_en_gpio =
    GPIO_DT_SPEC_GET(
        DT_ALIAS(motor_en),
        gpios);

/*
 * PA8 -> STEP PWM
 */
const pwm_dt_spec g_step_pwm =
    PWM_DT_SPEC_GET(
        DT_ALIAS(step_pwm));

constexpr std::uint32_t kStepPwmPeriodUs = 1000U;
constexpr std::uint32_t kStepPwmPulseUs  = 500U;

/*
 * ============================================================
 * CAN Hardware
 * ============================================================
 */

const device* g_can_dev =
    DEVICE_DT_GET(
        DT_NODELABEL(fdcan1));

constexpr std::uint32_t kStopMotorCanId  = 0x100U;
constexpr std::uint32_t kStartMotorCanId = 0x400U;

can_filter g_motor_filter
{
    .id = 0U,
    .mask = 0U,
    .flags = 0U
};

int g_filter_id = -1;

/*
 * ============================================================
 * CAN RX Callback
 * ============================================================
 */

void canRxCallback(
    const device*,
    can_frame* frame,
    void*)
{
    if (frame == nullptr)
    {
        return;
    }

    CanFd::processReceivedMessage(
        frame->id,
        frame->data,
        frame->dlc);
}

} // namespace

namespace motor_node
{

bool
GpioOverlay::init()
{
    if (!motorInit())
    {
        return false;
    }

    if (!canInit())
    {
        return false;
    }

    return true;
}

/*
 * ============================================================
 * Motor Driver
 * ============================================================
 */

bool
GpioOverlay::motorInit()
{
    if (!gpio_is_ready_dt(
            &g_dir_gpio))
    {
        return false;
    }

    if (!gpio_is_ready_dt(
            &g_en_gpio))
    {
        return false;
    }

    if (!pwm_is_ready_dt(
            &g_step_pwm))
    {
        return false;
    }

    if (gpio_pin_configure_dt(
            &g_dir_gpio,
            GPIO_OUTPUT_ACTIVE) != 0)
    {
        return false;
    }

    if (gpio_pin_configure_dt(
            &g_en_gpio,
            GPIO_OUTPUT_ACTIVE) != 0)
    {
        return false;
    }

    /*
     * Clockwise Direction
     */
    gpio_pin_set_dt(
        &g_dir_gpio,
        1);

    /*
     * Motor Disabled
     * EN = HIGH
     */
    gpio_pin_set_dt(
        &g_en_gpio,
        1);

    stopStepPwm();

    return true;
}

void
GpioOverlay::setMotorDirectionClockwise()
{
    gpio_pin_set_dt(
        &g_dir_gpio,
        1);
}

void
GpioOverlay::enableMotor()
{
    /*
     * TMC2209
     * LOW = Enable
     */
    gpio_pin_set_dt(
        &g_en_gpio,
        0);
}

void
GpioOverlay::disableMotor()
{
    /*
     * TMC2209
     * HIGH = Disable
     */
    gpio_pin_set_dt(
        &g_en_gpio,
        1);
}

bool
GpioOverlay::startStepPwm()
{
    return pwm_set_dt(
               &g_step_pwm,
               PWM_USEC(
                   kStepPwmPeriodUs),
               PWM_USEC(
                   kStepPwmPulseUs)) == 0;
}

void
GpioOverlay::stopStepPwm()
{
    (void)pwm_set_dt(
        &g_step_pwm,
        PWM_USEC(
            kStepPwmPeriodUs),
        0U);
}

/*
 * ============================================================
 * CAN
 * ============================================================
 */

bool
GpioOverlay::canInit()
{
    if (!device_is_ready(
            g_can_dev))
    {
        return false;
    }

    g_filter_id =
        can_add_rx_filter(
            g_can_dev,
            canRxCallback,
            nullptr,
            &g_motor_filter);

    if (g_filter_id < 0)
    {
        return false;
    }

    return can_start(
               g_can_dev) == 0;
}

bool
GpioOverlay::canTransmit(
    const std::uint32_t id,
    const std::uint8_t* data,
    const std::uint8_t length)
{
    can_frame frame {};

    frame.id = id;
    frame.dlc = length;
    frame.flags = 0U;

    for (std::uint8_t i = 0U;
         i < length;
         ++i)
    {
        frame.data[i] = data[i];
    }

    return can_send(
               g_can_dev,
               &frame,
               K_MSEC(10),
               nullptr,
               nullptr) == 0;
}

} // namespace motor_node