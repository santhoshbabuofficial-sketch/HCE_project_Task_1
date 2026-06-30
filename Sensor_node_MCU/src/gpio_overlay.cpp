#include "gpio_overlay.hpp"
#include "canfd.hpp"

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/can.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>

#include <cstdint>

// ============================================================
// ANONYMOUS NAMESPACE — TRANSLATION-UNIT-LOCAL STATE
// ============================================================
namespace
{

// ============================================================
// ADC CONFIGURATION (PRESSURE SENSOR — ADC1 CH1 / PA0)
// ============================================================

constexpr std::uint8_t  kPressureAdcChannel = 1U;
constexpr std::uint8_t  kAdcResolutionBits  = 12U;

const device* const g_adc_dev =
    DEVICE_DT_GET(DT_NODELABEL(adc1));

/*
 * FIX: adc_sequence::buffer must be uint16_t for 12-bit resolution.
 * Using int16_t was a sign-mismatch; Zephyr stores raw counts unsigned.
 */
std::uint16_t g_adc_sample = 0U;

const adc_channel_cfg g_adc_channel_cfg
{
    .gain             = ADC_GAIN_1,
    .reference        = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id       = kPressureAdcChannel,
    .differential     = 0U,
};

adc_sequence g_adc_sequence
{
    .options     = nullptr,
    .channels    = BIT(kPressureAdcChannel),
    .buffer      = &g_adc_sample,
    .buffer_size = sizeof(g_adc_sample),
    .resolution  = kAdcResolutionBits,
    .oversampling = 0U,
    .calibrate   = false,
};

// ============================================================
// FLOW SENSOR (PC2 — Rising-edge interrupt)
// ============================================================

/*
 * FIX: DT_ALIAS requires the alias name with underscores replaced by
 * hyphens exactly matching the DTS alias key "flow-sensor".
 * GPIO_DT_SPEC_GET resolves to the gpios property of that alias node.
 */
const gpio_dt_spec g_flow_gpio =
    GPIO_DT_SPEC_GET(DT_ALIAS(flow_sensor), gpios);

gpio_callback g_flow_callback;

/*
 * FIX: Use atomic_t (Zephyr) for the ISR-updated counter so that
 * the main thread reads a consistent value without disabling IRQs.
 * atomic_t is a struct wrapping a volatile int internally; all
 * access goes through atomic_get / atomic_inc.
 */
atomic_t g_flow_pulse_count = ATOMIC_INIT(0);

/*
 * FIX: ISR callback signature must match gpio_callback_handler_t exactly.
 * The function must NOT be noexcept in C (Zephyr C API), but because we
 * compile with C++ we mark it extern "C" to suppress name-mangling and
 * match the C function-pointer type expected by gpio_init_callback.
 */
extern "C" void flowIsr(
    const device* /*dev*/,
    gpio_callback* /*cb*/,
    std::uint32_t  /*pins*/)
{
    atomic_inc(&g_flow_pulse_count);
}

// ============================================================
// LCD (I2C1 — PCF8574 I2C backpack)
// ============================================================

constexpr std::uint16_t kLcdAddress   = 0x27U;

constexpr std::uint8_t  kLcdRs        = 0x01U;
constexpr std::uint8_t  kLcdEn        = 0x04U;
constexpr std::uint8_t  kLcdBacklight = 0x08U;

const device* const g_i2c_dev =
    DEVICE_DT_GET(DT_NODELABEL(i2c1));

// ============================================================
// HEARTBEAT LED (PC4 — Active HIGH)
// ============================================================

/*
 * FIX: alias key in DTS is "heartbeat-led" → DT_ALIAS(heartbeat_led).
 */
const gpio_dt_spec g_heartbeat_led =
    GPIO_DT_SPEC_GET(DT_ALIAS(heartbeat_led), gpios);

/// Drive the heartbeat LED to @p state (true = ON).
void heartbeatLedWrite(const bool state) noexcept
{
    if (!gpio_is_ready_dt(&g_heartbeat_led))
    {
        return;
    }

    (void)gpio_pin_set_dt(
        &g_heartbeat_led,
        static_cast<int>(state));
}

// ============================================================
// LCD LOW-LEVEL HELPERS (I2C 4-bit mode)
// ============================================================

bool lcdWriteByte(const std::uint8_t value) noexcept
{
    /*
     * FIX: i2c_write takes a non-const pointer to device; add cast.
     * Also cast address to uint16_t to silence implicit conversion warning.
     */
    return (i2c_write(
                g_i2c_dev,
                &value,
                1U,
                static_cast<std::uint16_t>(kLcdAddress)) == 0);
}

void lcdPulseEnable(const std::uint8_t value) noexcept
{
    (void)lcdWriteByte(static_cast<std::uint8_t>(value | kLcdEn));
    k_sleep(K_USEC(1));
    (void)lcdWriteByte(static_cast<std::uint8_t>(
        value & static_cast<std::uint8_t>(~kLcdEn)));
    k_sleep(K_USEC(50));
}

void lcdWrite4Bits(const std::uint8_t value) noexcept
{
    (void)lcdWriteByte(value);
    lcdPulseEnable(value);
}

void lcdSendByte(const std::uint8_t value, const bool is_data) noexcept
{
    const std::uint8_t rs    = is_data ? kLcdRs : 0U;
    const std::uint8_t upper = static_cast<std::uint8_t>(value & 0xF0U);
    const std::uint8_t lower = static_cast<std::uint8_t>((value << 4U) & 0xF0U);

    lcdWrite4Bits(static_cast<std::uint8_t>(upper | rs | kLcdBacklight));
    lcdWrite4Bits(static_cast<std::uint8_t>(lower | rs | kLcdBacklight));
}

// ============================================================
// CAN DEVICE (FDCAN1)
// ============================================================

const device* const g_can_dev =
    DEVICE_DT_GET(DT_NODELABEL(fdcan1));

constexpr std::uint32_t kEstopCanId = 0x101U;

/*
 * FIX: can_filter::flags must include CAN_FILTER_DATA for a standard
 * data-frame filter. Without it the driver may reject the filter.
 */
can_filter g_estop_filter
{
    .id    = kEstopCanId,
    .mask  = CAN_STD_ID_MASK,
    /*
     * FIX: CAN_FILTER_DATA was removed in Zephyr 3.x — standard data
     * frames are matched with flags = 0 (no FD, no RTR, no IDE).
     */
    .flags = static_cast<std::uint8_t>(0U),
};

int g_filter_id = -1;

// ============================================================
// CAN RX CALLBACK
// ============================================================

/*
 * FIX: Callback type is can_rx_callback_t which is a plain C function
 * pointer.  Mark extern "C" so the compiler generates a compatible
 * calling convention and the cast inside can_add_rx_filter is valid.
 */
extern "C" void canRxCallback(
    const device* /*dev*/,
    can_frame*    frame,
    void*         /*user_data*/)
{
    if (frame == nullptr)
    {
        return;
    }

    sensor_node::CanFd::processReceivedMessage(
        frame->id,
        frame->data,
        static_cast<std::uint8_t>(frame->dlc));
}

} // anonymous namespace

// ============================================================
// sensor_node::GpioOverlay IMPLEMENTATION
// ============================================================
namespace sensor_node
{

// ------------------------------------------------------------
// init() — peripherals
// ------------------------------------------------------------
bool GpioOverlay::init() noexcept
{
    // --------------------------------------------------------
    // ADC (PRESSURE SENSOR)
    // --------------------------------------------------------
    if (!device_is_ready(g_adc_dev))
    {
        return false;
    }

    if (adc_channel_setup(g_adc_dev, &g_adc_channel_cfg) != 0)
    {
        return false;
    }

    // --------------------------------------------------------
    // FLOW SENSOR GPIO (PC2 — Rising-edge interrupt)
    // --------------------------------------------------------
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

    // --------------------------------------------------------
    // HEARTBEAT LED (PC4)
    // --------------------------------------------------------
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

    return true;
}

// ------------------------------------------------------------
// canInit() — CAN-FD controller startup
// ------------------------------------------------------------

/*
 * FIX: canInit() was defined inside GpioOverlay::init()'s closing brace
 * in the original file — it was accidentally nested, making it an
 * undefined symbol at link time.  It is now a proper top-level method.
 */
bool GpioOverlay::canInit() noexcept
{
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

// ------------------------------------------------------------
// lcdInit()
// ------------------------------------------------------------
bool GpioOverlay::lcdInit() noexcept
{
    if (!device_is_ready(g_i2c_dev))
    {
        return false;
    }

    k_sleep(K_MSEC(50));

    lcdWrite4Bits(static_cast<std::uint8_t>(0x30U | kLcdBacklight));
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(static_cast<std::uint8_t>(0x30U | kLcdBacklight));
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(static_cast<std::uint8_t>(0x30U | kLcdBacklight));
    k_sleep(K_MSEC(5));

    lcdWrite4Bits(static_cast<std::uint8_t>(0x20U | kLcdBacklight));
    k_sleep(K_MSEC(5));

    lcdSendByte(0x28U, false);
    lcdSendByte(0x0CU, false);
    lcdSendByte(0x06U, false);

    lcdSendByte(0x01U, false);
    k_sleep(K_MSEC(2));

    return true;
}

// ------------------------------------------------------------
// lcdClear()
// ------------------------------------------------------------
void GpioOverlay::lcdClear() noexcept
{
    lcdSendByte(0x01U, false);
    k_sleep(K_MSEC(2));
}

// ------------------------------------------------------------
// lcdSetCursor()
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// lcdPrint()
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// readPressureAdcRaw()
// ------------------------------------------------------------
std::uint16_t GpioOverlay::readPressureAdcRaw() noexcept
{
    if (adc_read(g_adc_dev, &g_adc_sequence) != 0)
    {
        return 0U;
    }

    return g_adc_sample;
}

// ------------------------------------------------------------
// getFlowPulseCount()
// ------------------------------------------------------------
std::uint32_t GpioOverlay::getFlowPulseCount() noexcept
{
    return static_cast<std::uint32_t>(
        atomic_get(&g_flow_pulse_count));
}

// ------------------------------------------------------------
// getAndResetFlowPulseCount()
// ------------------------------------------------------------
std::uint32_t GpioOverlay::getAndResetFlowPulseCount() noexcept
{
    /*
     * FIX: Use atomic_set to clear after atomic_get so the ISR
     * cannot increment between the read and the clear.
     * atomic_clear sets to 0 and returns the old value — ideal here.
     */
    return static_cast<std::uint32_t>(
        atomic_clear(&g_flow_pulse_count));
}

// ------------------------------------------------------------
// canTransmit()
// ------------------------------------------------------------
bool GpioOverlay::canTransmit(
    std::uint32_t       id,
    const std::uint8_t* data,
    std::uint8_t        length) noexcept
{
    if ((data == nullptr) || (length == 0U))
    {
        return false;
    }

    can_frame frame {};

    frame.id    = id;
    frame.dlc   = length;
    /*
     * FIX: can_frame_flags was renamed/removed in newer Zephyr.
     * Cast to the underlying uint8_t type directly.
     */
    frame.flags = static_cast<std::uint8_t>(0U);

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

// ------------------------------------------------------------
// heartbeatLedPulse()
// ------------------------------------------------------------
void GpioOverlay::heartbeatLedPulse() noexcept
{
    if (!gpio_is_ready_dt(&g_heartbeat_led))
    {
        return;
    }

    heartbeatLedWrite(true);
    k_sleep(K_MSEC(500));
    heartbeatLedWrite(false);
}

} // namespace sensor_node