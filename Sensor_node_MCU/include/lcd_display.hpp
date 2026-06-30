#pragma once

#include <cstdint>

namespace sensor_node
{

/**
 * @brief LCD display controller (16×2 via PCF8574 I2C backpack).
 *
 * Handles normal sensor display and emergency E-STOP overlay.
 * Once showMessage() is called the display is frozen on the
 * emergency text; normal update() calls are ignored.
 *
 * All methods are static; this class cannot be instantiated.
 * No heap allocation is used.
 */
class LcdDisplay
{
public:

    /**
     * @brief Initialise the LCD hardware via I2C.
     *
     * Must be called once at startup before update() or showMessage().
     *
     * @return true  LCD responded and was configured successfully.
     * @return false I2C bus or device initialisation failed.
     */
    [[nodiscard]] static bool init() noexcept;

    /**
     * @brief Refresh the LCD with the latest sensor values.
     *
     * Line 0: "P:<value> mmHg"
     * Line 1: "F:<value> mL/min"
     *
     * Silently returns if an E-STOP message is currently displayed.
     */
    static void update() noexcept;

    /**
     * @brief Display an emergency system message and freeze normal updates.
     *
     * After this call, update() becomes a no-op until the system resets.
     *
     * @param text Null-terminated string to display on line 0.
     *             Must not be nullptr.
     */
    static void showMessage(const char* text) noexcept;

private:

    /// @cond PRIVATE
    LcdDisplay() = delete;
    /// @endcond
};

} // namespace sensor_node