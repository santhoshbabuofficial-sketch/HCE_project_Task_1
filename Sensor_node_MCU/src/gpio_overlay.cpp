#include "gpio_overlay.hpp"
#include "canfd.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include <cstdint>

namespace
{

// ============================================================
// ADC CONFIGURATION (PRESSURE SENSOR)
// ============================================================

constexpr std::uint8_t kPressureAdcChannel = 1U;
constexpr std::uint8_t kAdcResolutionBits  = 12U;

const device* const g_adc_dev =
    DEVICE_DT_GET(DT_NODELABEL(adc1));

std::int16_t g_adc_sample = 0;

const adc_channel_cfg g_adc_channel_cfg
{
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = kPressureAdcChannel,
};

adc_sequence g_adc_sequence
{
    .channels    = BIT(kPressureAdcChannel),
    .buffer      = &g_adc_sample,
    .buffer_size = sizeof(g_adc_sample),
    .resolution  = kAdcResolutionBits
};

// ============================================================
// FLOW SENSOR (PC2 INTERRUPT)
// ============================================================

const gpio_dt_spec g_flow_gpio =
    GPIO_DT_SPEC_GET(DT_ALIAS(flow_sensor), gpios);

gpio_callback g_flow_callback;

volatile std::uint32_t g_flow_pulse_count = 0U;

void flowIsr(
    const device* /*dev*/,
    gpio_callback* /*cb*/,
    std::uint32_t /*pins*/)
{
    ++g_flow_pulse_count;
}

// ============================================================
// LCD (I2C1 - PCF8574)
// ============================================================

constexpr std::uint16_t kLcdAddress = 0x27U;

constexpr std::uint8_t kLcdRs        = 0x01U;
constexpr std::uint8_t kLcdEn        = 0x04U;
constexpr std::uint8_t kLcdBacklight = 0x08U;

const device* const g_i2c_dev =
    DEVICE_DT_GET(DT_NODELABEL(i2c1));

// ============================================================
// HEARTBEAT LED (PC4)
// ============================================================

const gpio_dt_spec g_heartbeat_led =
    GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);

void heartbeatLedWrite(const bool state)
{
    if (!gpio_is_ready_dt(&g_heartbeat_led))
    {
        return;
    }

    (void)gpio_pin_set_dt(
        &g_heartbeat_led,
        static_cast<int>(state));
}

} // namespace
namespace
{

// ============================================================
// LCD LOW LEVEL HELPERS (I2C 4-bit MODE)
// ============================================================

bool lcdWriteByte(const std::uint8_t value)
{
    return (i2c_write(
                g_i2c_dev,
                &value,
                1U,
                kLcdAddress) == 0);
}

void lcdPulseEnable(const std::uint8_t value)
{
    (void)lcdWriteByte(value | kLcdEn);
    k_sleep(K_USEC(1));
    (void)lcdWriteByte(value & static_cast<std::uint8_t>(~kLcdEn));
    k_sleep(K_USEC(50));
}

void lcdWrite4Bits(const std::uint8_t value)
{
    (void)lcdWriteByte(value);
    lcdPulseEnable(value);
}

void lcdSendByte(const std::uint8_t value, const bool is_data)
{
    const std::uint8_t rs = is_data ? kLcdRs : 0U;

    const std::uint8_t upper = (value & 0xF0U);
    const std::uint8_t lower = ((value << 4U) & 0xF0U);

    lcdWrite4Bits(upper | rs | kLcdBacklight);
    lcdWrite4Bits(lower | rs | kLcdBacklight);
}

// ============================================================
// CAN DEVICE (FDCAN1)
// ============================================================

const device* const g_can_dev =
    DEVICE_DT_GET(DT_NODELABEL(fdcan1));

constexpr std::uint32_t kEstopCanId = 0x101U;

can_filter g_estop_filter
{
    .id    = kEstopCanId,
    .mask  = CAN_STD_ID_MASK,
    .flags = 0U
};

int g_filter_id = -1;

// Forward declaration
void canRxCallback(
    const device* dev,
    can_frame* frame,
    void* user_data);

} // namespace

namespace
{

// ============================================================
// CAN RX CALLBACK
// ============================================================

void canRxCallback(
    const device* /*dev*/,
    can_frame* frame,
    void* /*user_data*/)
{
    if (frame == nullptr)
    {
        return;
    }

    sensor_node::CanFd::processReceivedMessage(
        frame->id,
        frame->data,
        frame->dlc);
}

} // namespace
namespace sensor_node
{

// ============================================================
// INITIALIZATION
// ============================================================

bool GpioOverlay::init() noexcept
{
    // ============================================================
    // ADC (PRESSURE SENSOR)
    // ============================================================

    if (!device_is_ready(g_adc_dev))
    {
        return false;
    }

    if (adc_channel_setup(g_adc_dev, &g_adc_channel_cfg) != 0)
    {
        return false;
    }

    // ============================================================
    // FLOW SENSOR GPIO (PC2 INTERRUPT)
// ============================================================

    if (!gpio_is_ready_dt(&g_flow_gpio))
    {
        return false;
    }

    if (gpio_pin_configure_dt(&g_flow_gpio, GPIO_INPUT) != 0)
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

    // ============================================================
    // HEARTBEAT LED (PC4)
    // ============================================================

    if (!gpio_is_ready_dt(&g_heartbeat_led))
    {
        return false;
    }

    if (gpio_pin_configure_dt(
            &g_heartbeat_led,
            GPIO_OUTPUT_ACTIVE) != 0)
    {
        return false;
    }

    heartbeatLedWrite(false);

    // ============================================================
    // CAN INIT + FILTER
    // ============================================================

    if (!device_is_ready(g_can_dev))
    {
        return false;
    }

    g_filter_id = can_add_rx_filter(
        g_can_dev,
        canRxCallback,
        nullptr,
        &g_estop_filter);

    if (g_filter_id < 0)
    {
        return false;
    }

    if (can_start(g_can_dev) != 0)
    {
        return false;
    }

    return true;
}
namespace sensor_node
{

// ============================================================
// LCD INITIALIZATION
// ============================================================

bool GpioOverlay::lcdInit() noexcept
{
    if (!device_is_ready(g_i2c_dev))
    {
        return false;
    }

    k_sleep(K_MSEC(50));

    lcdWrite4Bits(0x30U | kLcdBacklight);
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(0x30U | kLcdBacklight);
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(0x30U | kLcdBacklight);
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(0x20U | kLcdBacklight);
    k_sleep(K_MSEC(5));

    lcdSendByte(0x28U, false);
    lcdSendByte(0x0CU, false);
    lcdSendByte(0x06U, false);

    lcdSendByte(0x01U, false);
    k_sleep(K_MSEC(2));

    return true;
}

// ============================================================
// LCD CONTROL
// ============================================================

void GpioOverlay::lcdClear() noexcept
{
    lcdSendByte(0x01U, false);
    k_sleep(K_MSEC(2));
}

void GpioOverlay::lcdSetCursor(
    std::uint8_t row,
    std::uint8_t col) noexcept
{
    std::uint8_t address = col;

    if (row == 1U)
    {
        address = static_cast<std::uint8_t>(address + 0x40U);
    }

    lcdSendByte(static_cast<std::uint8_t>(0x80U | address), false);
}

void GpioOverlay::lcdPrint(const char* text) noexcept
{
    if (text == nullptr)
    {
        return;
    }

    while (*text != '\0')
    {
        lcdSendByte(static_cast<std::uint8_t>(*text), true);
        ++text;
    }
}

// ============================================================
// PRESSURE SENSOR (ADC READ)
// ============================================================

std::uint16_t GpioOverlay::readPressureAdcRaw() noexcept
{
    if (adc_read(g_adc_dev, &g_adc_sequence) != 0)
    {
        return 0U;
    }

    return static_cast<std::uint16_t>(g_adc_sample);
}

// ============================================================
// FLOW SENSOR APIs
// ============================================================

std::uint32_t GpioOverlay::getFlowPulseCount() noexcept
{
    return g_flow_pulse_count;
}

std::uint32_t GpioOverlay::getAndResetFlowPulseCount() noexcept
{
    const std::uint32_t count = g_flow_pulse_count;
    g_flow_pulse_count = 0U;
    return count;
}

} // namespace sensor_node
namespace sensor_node
{

// ============================================================
// CAN TRANSMIT
// ============================================================

bool GpioOverlay::canTransmit(
    std::uint32_t id,
    const std::uint8_t* data,
    std::uint8_t length) noexcept
{
    if ((data == nullptr) || (length == 0U))
    {
        return false;
    }

    can_frame frame {};

    frame.id = id;
    frame.dlc = length;
    frame.flags = 0U;

    for (std::uint8_t i = 0U; i < length; ++i)
    {
        frame.data[i] = data[i];
    }

    return (can_send(
                g_can_dev,
                &frame,
                K_MSEC(10),
                nullptr,
                nullptr) == 0);
}

// ============================================================
// HEARTBEAT LED PULSE (PC4 - ACTIVE HIGH)
// ============================================================

void GpioOverlay::heartbeatLedPulse() noexcept
{
    if (!gpio_is_ready_dt(&g_heartbeat_led))
    {
        return;
    }

    // ON
    heartbeatLedWrite(true);

    // HOLD FOR 500ms (USER REQUIREMENT)
    k_sleep(K_MSEC(500));

    // OFF
    heartbeatLedWrite(false);
}

} // namespace sensor_node