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

    /**
     * @brief Display a custom message.
     *
     * Used for supervisor commands such as:
     * "E Stop Broadcasting"
     */
    static void showMessage(
        const char* text);

private:
    LcdDisplay() = delete;
};

} // namespace sensor_node