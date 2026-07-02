/// @file lcd_display.hpp
/// @brief Driver for a 16x2 HD44780 LCD behind a PCF8574 I2C backpack.
#pragma once

#include <cstdint>

#include "gpio_overlay.hpp"

namespace hce::sensor_node {

/// @brief Interface so LcdDisplay can be mocked on the host build, and so
///        SensorManager can notify it via the Observer pattern.
class ILcdDisplay {
public:
    virtual ~ILcdDisplay() = default;
    virtual bool Init() = 0;
    virtual void ShowPressure(float pressure_pa) = 0;
    virtual void ShowFlow(float flow_lpm) = 0;
    virtual void ShowRunning() = 0;
    virtual void ShowStopped() = 0;
    virtual void ShowEmergencyStop() = 0;
};

/// @brief PCF8574-backed 16x2 LCD driver (I2C1).
class LcdDisplay final : public ILcdDisplay {
public:
    explicit LcdDisplay(GpioOverlay& overlay);

    bool Init() override;
    void ShowPressure(float pressure_pa) override;
    void ShowFlow(float flow_lpm) override;
    void ShowRunning() override;
    void ShowStopped() override;
    void ShowEmergencyStop() override;

private:
    static constexpr uint8_t kLcdColumns = 16U;
    static constexpr uint8_t kLcdRows = 2U;
    static constexpr uint8_t kBacklightBit = 0x08U;
    static constexpr uint8_t kEnableBit = 0x04U;
    static constexpr uint8_t kRegisterSelectBit = 0x01U;

    /// @brief Write a single nibble + control bits to the PCF8574 expander.
    void WriteNibble(uint8_t nibble, bool is_data);
    /// @brief Write a full byte (two nibbles).
    void WriteByte(uint8_t value, bool is_data);
    void SetCursor(uint8_t row, uint8_t col);
    void WriteLine(uint8_t row, const char* text);

    GpioOverlay& overlay_;
    bool initialized_ = false;
};

}  // namespace hce::sensor_node
