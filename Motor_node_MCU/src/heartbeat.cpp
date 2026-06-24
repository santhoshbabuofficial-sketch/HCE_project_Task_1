#include "heartbeat.hpp"

#include "canfd.hpp"

namespace motor_node
{

bool
Heartbeat::init()
{
    return true;
}

void
Heartbeat::update()
{
    CanFd::sendHeartbeat();
}

} // namespace motor_node