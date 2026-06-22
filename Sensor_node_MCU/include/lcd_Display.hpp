#pragma once

namespace sensor_node
{

class LcdDisplay
{
public:
    /**
     * @brief Initialize LCD display.
     */
    static bool init();

    /**
     * @brief Update LCD with latest sensor values.
     */
    static void update();

private:
    LcdDisplay() = delete;
};

} // namespace sensor_node