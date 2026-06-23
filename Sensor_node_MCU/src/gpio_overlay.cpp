#include "gpio_overlay.hpp"

#include "canfd.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

namespace
{

/*
 * ============================================================
 * ADC Configuration
 * ============================================================
 */
constexpr std::uint8_t kPressureAdcChannel = 1U;
constexpr std::uint8_t kAdcResolution = 12U;

const device* g_adc_dev =
    DEVICE_DT_GET(DT_NODELABEL(adc1));

std::int16_t g_adc_sample = 0;

adc_channel_cfg g_adc_channel_cfg
{
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = kPressureAdcChannel,
};

adc_sequence g_adc_sequence
{
    .channels = BIT(kPressureAdcChannel),
    .buffer = &g_adc_sample,
    .buffer_size = sizeof(g_adc_sample),
    .resolution = kAdcResolution
};

/*
 * ============================================================
 * Flow Sensor
 * PC2
 * ============================================================
 */
const gpio_dt_spec g_flow_gpio =
    GPIO_DT_SPEC_GET(
        DT_ALIAS(flow_sensor),
        gpios);

gpio_callback g_flow_callback;

volatile std::uint32_t g_flow_pulse_count = 0U;

void flowIsr(
    const device*,
    gpio_callback*,
    std::uint32_t)
{
    ++g_flow_pulse_count;
}

/*
 * ============================================================
 * LCD I2C
 * PB8 -> SCL
 * PB9 -> SDA
 * ============================================================
 */
constexpr std::uint16_t kLcdAddress = 0x27U;

constexpr std::uint8_t kRs = 0x01U;
constexpr std::uint8_t kEn = 0x04U;
constexpr std::uint8_t kBacklight = 0x08U;

const device* g_i2c_dev =
    DEVICE_DT_GET(DT_NODELABEL(i2c1));
    /*
 * ============================================================
 * LCD Low Level Functions
 * ============================================================
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
    (void)lcdWriteByte(
        value | kEn);

    k_sleep(K_USEC(1));

    (void)lcdWriteByte(
        value &
        static_cast<std::uint8_t>(~kEn));

    k_sleep(K_USEC(50));
}

void lcdWrite4Bits(
    const std::uint8_t value)
{
    (void)lcdWriteByte(value);

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
    const std::uint8_t command)
{
    lcdSendByte(
        command,
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
 * ============================================================
 * CAN Hardware
 * FDCAN1
 * PA11 -> RX
 * PA12 -> TX
 * ============================================================
 */

const device* g_can_dev =
    DEVICE_DT_GET(DT_NODELABEL(fdcan1));

constexpr std::uint32_t kEstopCanId = 0x101U;

can_filter g_estop_filter
{
    .id = kEstopCanId,
    .mask = CAN_STD_ID_MASK,
    .flags = 0U
};

int g_filter_id = -1;

/*
 * Forward declaration
 */
void canRxCallback(
    const device* dev,
    can_frame* frame,
    void* user_data);
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

namespace sensor_node
{

bool
GpioOverlay::init()
{
    /*
     * ----------------------------
     * ADC
     * ----------------------------
     */
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

    /*
     * ----------------------------
     * Flow Sensor
     * ----------------------------
     */
    if (!gpio_is_ready_dt(
            &g_flow_gpio))
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
bool
GpioOverlay::lcdInit()
{
    if (!device_is_ready(
            g_i2c_dev))
    {
        return false;
    }

    k_sleep(K_MSEC(50));

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

    lcdCommand(0x28U);
    lcdCommand(0x0CU);
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
    std::uint8_t address = col;

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
            &g_estop_filter);

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