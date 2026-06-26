#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief LCD display controller.
 *
 * Handles sensor display and emergency messages.
 */
class LcdDisplay
{
public:

    /**
     * @brief Initialize LCD hardware.
     */
    static bool init() noexcept;

    /**
     * @brief Update LCD with sensor values.
     */
    static void update() noexcept;

    /**
     * @brief Display system message (e.g., E-Stop).
     */
    static void showMessage(const char* text) noexcept;

private:
    LcdDisplay() = delete;
};

} // namespace sensor_node