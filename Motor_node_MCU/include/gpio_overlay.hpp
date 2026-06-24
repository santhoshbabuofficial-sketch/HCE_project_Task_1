#pragma once

#include <cstdint>

namespace motor_node
{

class GpioOverlay
{
public:
    /*
     * ==========================================================
     * Initialization
     * ==========================================================
     */
    static bool init();

    /*
     * ==========================================================
     * Stepper Driver Hardware Access
     *
     * PA8 -> STEP (PWM)
     * PB0 -> DIR
     * PB1 -> EN
     * ==========================================================
     */
    static bool motorInit();

    static void setMotorDirectionClockwise();

    static void enableMotor();

    static void disableMotor();

    static bool startStepPwm();

    static void stopStepPwm();

    /*
     * ==========================================================
     * CAN Hardware Access
     *
     * FDCAN1
     * PA11 -> RX
     * PA12 -> TX
     * ==========================================================
     */
    static bool canInit();

    static bool canTransmit(
        std::uint32_t id,
        const std::uint8_t* data,
        std::uint8_t length);

private:
    GpioOverlay() = delete;
};

} // namespace motor_node