/// @file main.cpp
/// @brief Motor Node entry point. Wires gpio_overlay -> stepper_motor_driver
///        -> motor_controller and drives it from CAN RUN/STOP commands.
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "can_bus_driver.hpp"
#include "gpio_overlay.hpp"
#include "heartbeat.hpp"
#include "message_codec.hpp"
#include "motor_controller.hpp"
#include "stepper_motor_driver.hpp"

LOG_MODULE_REGISTER(motor_node_main, LOG_LEVEL_INF);

namespace {

using hce::motor_node::GpioOverlay;
using hce::motor_node::Heartbeat;
using hce::motor_node::MotorController;
using hce::motor_node::StepperMotorDriver;

constexpr uint32_t kStepTickPeriodUs = 1000U;
constexpr size_t kStepThreadStackSize = 1024U;
constexpr size_t kHeartbeatThreadStackSize = 1024U;
constexpr int kStepThreadPriority = 4;  // Higher priority: timing-critical.
constexpr int kHeartbeatThreadPriority = 5;

GpioOverlay g_overlay;
hce::motor::CanBusDriver* g_can_bus = nullptr;
StepperMotorDriver* g_stepper_driver = nullptr;
MotorController* g_motor_controller = nullptr;
Heartbeat* g_heartbeat = nullptr;

alignas(hce::motor::CanBusDriver) uint8_t g_can_bus_storage[sizeof(hce::motor::CanBusDriver)];
alignas(StepperMotorDriver) uint8_t g_stepper_storage[sizeof(StepperMotorDriver)];
alignas(MotorController) uint8_t g_controller_storage[sizeof(MotorController)];
alignas(Heartbeat) uint8_t g_heartbeat_storage[sizeof(Heartbeat)];

/// @brief ISR-context CAN RX callback for STOP (0x100). GPIO writes are
///        register-level and safe to issue directly from ISR context, so
///        no deferred work item is required for this safety-critical path.
extern "C" void OnStopCommandRx(const device* /*dev*/, can_frame* /*frame*/, void* /*user_data*/) {
    g_motor_controller->OnStopCommand();
}

/// @brief ISR-context CAN RX callback for RUN (0x400).
extern "C" void OnRunCommandRx(const device* /*dev*/, can_frame* frame, void* /*user_data*/) {
    hce::motor::MessageCodec::Payload payload{};
    for (uint8_t i = 0U; i < hce::motor::MessageCodec::kPayloadBytes && i < frame->dlc; ++i) {
        payload[i] = frame->data[i];
    }
    const hce::motor::RunCommandMessage msg = hce::motor::MessageCodec::DecodeRunCommand(payload);
    g_motor_controller->OnRunCommand(msg.step_rate_hz, msg.direction == 0U);
}

void StepThreadEntry(void* /*p1*/, void* /*p2*/, void* /*p3*/) {
    while (true) {
        g_motor_controller->Tick();
        k_usleep(kStepTickPeriodUs);
    }
}

void HeartbeatThreadEntry(void* /*p1*/, void* /*p2*/, void* /*p3*/) {
    while (true) {
        g_heartbeat->Tick();
        k_msleep(Heartbeat::kPeriodMs);
    }
}

K_THREAD_STACK_DEFINE(g_step_thread_stack, kStepThreadStackSize);
K_THREAD_STACK_DEFINE(g_heartbeat_thread_stack, kHeartbeatThreadStackSize);
k_thread g_step_thread{};
k_thread g_heartbeat_thread{};

}  // namespace

int main() {
    LOG_INF("Motor Node booting");

    if (!g_overlay.Init()) {
        LOG_ERR("gpio_overlay init failed - halting");
        return -1;
    }

    g_can_bus = new (g_can_bus_storage) hce::motor::CanBusDriver(g_overlay.CanDevice());
    g_stepper_driver = new (g_stepper_storage) StepperMotorDriver(g_overlay);
    g_motor_controller = new (g_controller_storage) MotorController(*g_stepper_driver);
    g_heartbeat = new (g_heartbeat_storage) Heartbeat(g_overlay, *g_can_bus);

    if (!g_can_bus->Init()) {
        LOG_ERR("CAN bus init failed - halting");
        return -1;
    }

    g_can_bus->AddRxFilter(hce::motor::CanId::kStopCommand, OnStopCommandRx, nullptr);
    g_can_bus->AddRxFilter(hce::motor::CanId::kRunCommand, OnRunCommandRx, nullptr);

    // Boot safe: motor starts disabled until an explicit RUN command arrives.
    g_motor_controller->OnStopCommand();

    k_thread_create(&g_step_thread, g_step_thread_stack, kStepThreadStackSize, StepThreadEntry,
                     nullptr, nullptr, nullptr, kStepThreadPriority, 0, K_NO_WAIT);
    k_thread_name_set(&g_step_thread, "motor_step");

    k_thread_create(&g_heartbeat_thread, g_heartbeat_thread_stack, kHeartbeatThreadStackSize,
                     HeartbeatThreadEntry, nullptr, nullptr, nullptr, kHeartbeatThreadPriority, 0,
                     K_NO_WAIT);
    k_thread_name_set(&g_heartbeat_thread, "motor_heartbeat");

    LOG_INF("Motor Node running");
    return 0;
}
