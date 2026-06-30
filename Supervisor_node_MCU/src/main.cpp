#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "gpio_overlay.hpp"
#include "canfd.hpp"
#include "heartbeat_monitoring.hpp"
#include "alarm.hpp"

LOG_MODULE_REGISTER(supervisor_node, LOG_LEVEL_INF);

/* ============================================================
 * SYSTEM LIMITS (NO MAGIC NUMBERS)
 * ============================================================
 */
static constexpr std::uint16_t kFlowMin_mL_min  = 350U;
static constexpr std::uint16_t kFlowMax_mL_min  = 450U;

static constexpr std::uint16_t kPressureMin_mmHg = 100U;
static constexpr std::uint16_t kPressureMax_mmHg = 180U;

/* ============================================================
 * SHARED SENSOR DATA (UPDATED VIA CAN RX)
 * ============================================================
 * NOTE: deliberately at TU (external) linkage, not inside an
 * unnamed namespace. canfd.cpp declares these `extern` and
 * writes to them from CanFd::DecodeRx() on CAN RX of the
 * pressure/flow frames; an unnamed namespace would give them
 * internal linkage and silently create a second, disconnected
 * copy per translation unit (link-time undefined reference).
 */
volatile std::uint16_t g_flow_value = 0U;
volatile std::uint16_t g_pressure_value = 0U;

/* ============================================================
 * THREAD STACKS
 * ============================================================
 */
K_THREAD_STACK_DEFINE(can_stack, 2048);
K_THREAD_STACK_DEFINE(safety_stack, 2048);
K_THREAD_STACK_DEFINE(hb_stack, 1024);

static struct k_thread can_thread;
static struct k_thread safety_thread;
static struct k_thread hb_thread;

/* ============================================================
 * CAN RX THREAD (100ms cycle)
 * ============================================================
 */
static void CanThread()
{
    LOG_INF("CAN thread started");

    CanFd::ProcessRx();
}

/* ============================================================
 * SAFETY EVALUATION THREAD (1 second cycle)
 * ============================================================
 * Checks:
 * - Flow range
 * - Pressure range
 * - Triggers STOP if out of range
 * ============================================================
 */
static void SafetyThread()
{
    LOG_INF("Safety thread started");

    while (true)
    {
        bool flow_fault =
            (g_flow_value < kFlowMin_mL_min) ||
            (g_flow_value > kFlowMax_mL_min);

        bool pressure_fault =
            (g_pressure_value < kPressureMin_mmHg) ||
            (g_pressure_value > kPressureMax_mmHg);

        if (flow_fault || pressure_fault)
        {
            LOG_ERR("Safety fault detected -> STOP MOTOR");

            CanFd::SendStopMotor();

            /* Out-of-range process value is a safety-critical fault:
             * latch PC4 ALARM until the next MCU reset. */
            Alarm::Trigger();
        }

        HeartbeatMonitoring::Evaluate();

        k_sleep(K_MSEC(1000));
    }
}

/* ============================================================
 * HEARTBEAT THREAD (1 second cycle)
 * ============================================================
 */
static void HeartbeatThread()
{
    LOG_INF("Heartbeat thread started");

    while (true)
    {
        /* If the alarm has latched, PC4 must stay solid ON and must
         * never be touched again until a hardware reset occurs. */
        if (!Alarm::IsLatched())
        {
            /* Future: LED blink + system alive monitoring */
        }

        k_sleep(K_MSEC(1000));
    }
}

/* ============================================================
 * MAIN
 * ============================================================
 */
int main()
{
    LOG_INF("Supervisor Node Booting...");

    /* INIT HARDWARE */
    if (!GpioOverlay::Init())
    {
        LOG_ERR("GPIO Init failed");
        return -1;
    }

    if (!Alarm::Init())
    {
        LOG_ERR("Alarm Init failed");
        return -1;
    }

    if (!CanFd::Init())
    {
        LOG_ERR("CAN Init failed");
        return -1;
    }

    HeartbeatMonitoring::Init();

    /* ========================================================
     * START THREADS
     * ========================================================
     */
    k_thread_create(&can_thread,
                    can_stack,
                    K_THREAD_STACK_SIZEOF(can_stack),
                    (k_thread_entry_t)CanThread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    k_thread_create(&safety_thread,
                    safety_stack,
                    K_THREAD_STACK_SIZEOF(safety_stack),
                    (k_thread_entry_t)SafetyThread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    k_thread_create(&hb_thread,
                    hb_stack,
                    K_THREAD_STACK_SIZEOF(hb_stack),
                    (k_thread_entry_t)HeartbeatThread,
                    nullptr, nullptr, nullptr,
                    5, 0, K_NO_WAIT);

    LOG_INF("Supervisor Node Running");

    while (true)
    {
        k_sleep(K_FOREVER);
    }

    return 0;
}