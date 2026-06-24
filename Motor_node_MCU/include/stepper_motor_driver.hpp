#pragma once

namespace motor_node
{

class StepperMotorDriver
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
     * Motor Control
     * ==========================================================
     */
    static void startMotor();

    static void stopMotor();

    static bool isMotorRunning();

private:
    StepperMotorDriver() = delete;

    static bool m_is_motor_running;
};

} // namespace motor_node