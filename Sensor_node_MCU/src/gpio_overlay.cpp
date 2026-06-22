#include "gpio_overlay.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

namespace
{

constexpr std::uint8_t kPressureAdcChannel = 1U;
constexpr std::uint8_t kAdcResolution = 12U;

/*
 * ADC1 device
 */
const device* g_adc_dev =
    DEVICE_DT_GET(DT_NODELABEL(adc1));

/*
 * Flow sensor from overlay alias
 */
const gpio_dt_spec g_flow_gpio =
    GPIO_DT_SPEC_GET(
        DT_ALIAS(flow_sensor),
        gpios);

gpio_callback g_flow_callback;

/*
 * ADC sample buffer
 */
std::int16_t g_adc_sample = 0;

/*
 * Pulse counter
 */
volatile std::uint32_t g_flow_pulse_count = 0U;

/*
 * ADC configuration
 */
adc_channel_cfg g_adc_channel_cfg
{
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = kPressureAdcChannel,
};

/*
 * ADC sequence
 */
adc_sequence g_adc_sequence
{
    .channels = BIT(kPressureAdcChannel),
    .buffer = &g_adc_sample,
    .buffer_size = sizeof(g_adc_sample),
    .resolution = kAdcResolution
};
/*
 * LCD I2C
 */
constexpr std::uint16_t kLcdAddress = 0x27U;

constexpr std::uint8_t kRs = 0x01U;
constexpr std::uint8_t kEn = 0x04U;
constexpr std::uint8_t kBacklight = 0x08U;

const device* g_i2c_dev =
    DEVICE_DT_GET(DT_NODELABEL(i2c1));



    /*
 * LCD Low-Level Functions
 */

bool lcdWriteByte(
    const std::uint8_t value)
{
    return i2c_write(
               g_i2c_dev,
               &value,
               1U,
               kLcdAddress) == 0;
}

void lcdPulseEnable(
    const std::uint8_t value)
{
    lcdWriteByte(
        value | kEn);

    k_sleep(K_USEC(1));

    lcdWriteByte(
        value &
        static_cast<std::uint8_t>(~kEn));

    k_sleep(K_USEC(50));
}

void lcdWrite4Bits(
    const std::uint8_t value)
{
    lcdWriteByte(value);

    lcdPulseEnable(value);
}

void lcdSendByte(
    const std::uint8_t value,
    const bool is_data)
{
    const std::uint8_t rs =
        is_data ? kRs : 0U;

    const std::uint8_t upper =
        static_cast<std::uint8_t>(
            value & 0xF0U);

    const std::uint8_t lower =
        static_cast<std::uint8_t>(
            (value << 4U) & 0xF0U);

    lcdWrite4Bits(
        upper |
        rs |
        kBacklight);

    lcdWrite4Bits(
        lower |
        rs |
        kBacklight);
}

void lcdCommand(
    const std::uint8_t cmd)
{
    lcdSendByte(
        cmd,
        false);
}

void lcdData(
    const std::uint8_t data)
{
    lcdSendByte(
        data,
        true);
}
/*
 * ISR
 */
void flowIsr(
    const device*,
    gpio_callback*,
    std::uint32_t)
{
    ++g_flow_pulse_count;
}




} // namespace

namespace sensor_node
{
bool
GpioOverlay::lcdInit()
{
    if (!device_is_ready(g_i2c_dev))
    {
        return false;
    }

    k_sleep(K_MSEC(50));

    /*
     * HD44780 4-bit initialization
     */
    lcdWrite4Bits(
        0x30U | kBacklight);

    k_sleep(K_MSEC(5));

    lcdWrite4Bits(
        0x30U | kBacklight);

    k_sleep(K_MSEC(5));

    lcdWrite4Bits(
        0x30U | kBacklight);

    k_sleep(K_MSEC(5));

    lcdWrite4Bits(
        0x20U | kBacklight);

    k_sleep(K_MSEC(5));

    /*
     * Function Set
     * 4-bit
     * 2-line
     */
    lcdCommand(0x28U);

    /*
     * Display ON
     */
    lcdCommand(0x0CU);

    /*
     * Entry Mode
     */
    lcdCommand(0x06U);

    lcdClear();

    return true;
}

void
GpioOverlay::lcdClear()
{
    lcdCommand(0x01U);

    k_sleep(K_MSEC(2));
}

void
GpioOverlay::lcdSetCursor(
    const std::uint8_t row,
    const std::uint8_t col)
{
    std::uint8_t address =
        col;

    if (row == 1U)
    {
        address =
            static_cast<std::uint8_t>(
                address + 0x40U);
    }

    lcdCommand(
        static_cast<std::uint8_t>(
            0x80U | address));
}

void
GpioOverlay::lcdPrint(
    const char* text)
{
    while (*text != '\0')
    {
        lcdData(
            static_cast<std::uint8_t>(
                *text));

        ++text;
    }
}
bool GpioOverlay::init()
{
    if (!device_is_ready(g_adc_dev))
    {
        return false;
    }

    if (adc_channel_setup(
            g_adc_dev,
            &g_adc_channel_cfg) != 0)
    {
        return false;
    }

    if (!gpio_is_ready_dt(&g_flow_gpio))
    {
        return false;
    }

    if (gpio_pin_configure_dt(
            &g_flow_gpio,
            GPIO_INPUT) != 0)
    {
        return false;
    }

    if (gpio_pin_interrupt_configure_dt(
            &g_flow_gpio,
            GPIO_INT_EDGE_RISING) != 0)
    {
        return false;
    }

    gpio_init_callback(
        &g_flow_callback,
        flowIsr,
        BIT(g_flow_gpio.pin));

    gpio_add_callback(
        g_flow_gpio.port,
        &g_flow_callback);

    return true;
}

std::uint16_t
GpioOverlay::readPressureAdcRaw()
{
    if (adc_read(
            g_adc_dev,
            &g_adc_sequence) != 0)
    {
        return 0U;
    }

    return static_cast<std::uint16_t>(
        g_adc_sample);
}

std::uint32_t
GpioOverlay::getFlowPulseCount()
{
    return g_flow_pulse_count;
}

std::uint32_t
GpioOverlay::getAndResetFlowPulseCount()
{
    const std::uint32_t count =
        g_flow_pulse_count;

    g_flow_pulse_count = 0U;

    return count;
}

} // namespace sensor_node