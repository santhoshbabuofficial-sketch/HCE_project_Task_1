#ifndef SENSOR_NODE_HEARTBEAT_HPP
#define SENSOR_NODE_HEARTBEAT_HPP

namespace sensor_node
{

class Heartbeat
{
public:
    static bool init();

    static void update();
};

} // namespace sensor_node

#endif // SENSOR_NODE_HEARTBEAT_HPP