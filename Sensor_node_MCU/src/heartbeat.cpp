#include "heartbeat.hpp"
#include "canfd.hpp"

namespace sensor_node
{

bool Heartbeat::init() noexcept
{
    // No hardware setup required currently
    return true;
}

void Heartbeat::update() noexcept
{
    CanFd::sendHeartbeat();
}

} // namespace sensor_node