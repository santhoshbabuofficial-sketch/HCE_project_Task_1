#pragma once

namespace motor_node
{

class Heartbeat
{
public:
    /*
     * ==========================================================
     * Initialize Heartbeat Module
     * ==========================================================
     */
    static bool init();

    /*
     * ==========================================================
     * CAN ID : 0x201
     * No Payload
     * ==========================================================
     */
    static void update();

private:
    Heartbeat() = delete;
};

} // namespace motor_node