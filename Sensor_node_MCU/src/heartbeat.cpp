#include "heartbeat.hpp"

#include "canfd.hpp"

namespace sensor_node
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

} // namespace sensor_node