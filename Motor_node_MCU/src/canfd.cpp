#include "canfd.hpp"

#include "gpio_overlay.hpp"
#include "stepper_motor_driver.hpp"

#include <cstdint>

namespace
{

/*
 * ============================================================
 * CAN IDs
 * ============================================================
 */
constexpr std::uint32_t kStopMotorCanId  = 0x100U;
constexpr std::uint32_t kStartMotorCanId = 0x400U;
constexpr std::uint32_t kHeartbeatCanId  = 0x201U;

} // namespace

namespace motor_node
{

bool
CanFd::init()
{
    return GpioOverlay::canInit();
}

void
CanFd::sendHeartbeat()
{
    transmit(
        kHeartbeatCanId,
        nullptr,
        0U);
}

void
CanFd::processReceivedMessage(
    const std::uint32_t id,
    const std::uint8_t*,
    const std::uint8_t)
{
    /*
     * --------------------------------------------------------
     * Supervisor -> Motor Node
     * 0x400 Start Motor
     * --------------------------------------------------------
     */
    if (id == kStartMotorCanId)
    {
        StepperMotorDriver::startMotor();
        return;
    }

    /*
     * --------------------------------------------------------
     * Supervisor -> Motor Node
     * 0x100 Stop Motor
     * --------------------------------------------------------
     */
    if (id == kStopMotorCanId)
    {
        StepperMotorDriver::stopMotor();
        return;
    }
}

void
CanFd::transmit(
    const std::uint32_t id,
    const std::uint8_t* data,
    const std::uint8_t length)
{
    (void)GpioOverlay::canTransmit(
        id,
        data,
        length);
}

} // namespace motor_node