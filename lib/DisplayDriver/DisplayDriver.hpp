#ifndef DISPLAY_DRIVER_HPP
#define DISPLAY_DRIVER_HPP

#include <Arduino.h>

struct Timer
{
  uint32_t start;
  uint32_t top;

  Timer()
  {
    reset();
  }

  void
  setTop(uint32_t _top)
  {
    top = _top;
    reset();
  }

  void
  reset()
  {
    start = micros();
  }

  bool
  overflow()
  {
    return micros() - start > top;
  }
};

struct DisplayConfig
{
  // Digits
  uint8_t digit[6];
  // Right dots
  bool rdot[6];
  // Left dots
  bool ldot[6];

  DisplayConfig()
  {
    reset();
  }

  void
  reset()
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      digit[i] = 0;
      rdot[i] = false;
      ldot[i] = false;
    }
  }

  void
  resetDots()
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      rdot[i] = false;
      ldot[i] = false;
    }

  }
};

class DisplayDriver
{
  public:
    DisplayDriver(uint8_t _pin_enabled, uint8_t _pin_str, uint8_t _pin_clk, uint8_t _pin_data, uint8_t _pin_led);

    //! Initialize the display
    void
    begin();

    //! Set display digits, by digit set
    //! @param digit_set_0 first digit set (leftmost 2 digits)
    //! @param digit_set_1 second digit set (center 2 digits)
    //! @param digit_set_2 third digit set (rightmost 2 digits)
    //! @return true if digits are set, false otherwise
    bool
    setDisplay(const uint8_t digit_set_0, const uint8_t digit_set_1, const uint8_t digit_set_2);

    //! Set dot to display
    //! @param digit digit to display dot (0 - 5)
    //! @return true dot is set, false otherwise
    bool
    setDot(const uint8_t digit);

    //! Main display 
    void
    run();

  private:
    //! Pins used for communicating with HV5122
    //! Enable pin
    uint8_t m_pin_enabled;
    //! Stobe pin
    uint8_t m_pin_str;
    //! Clock pin
    uint8_t m_pin_clk;
    //! Data pin
    uint8_t m_pin_data;
    //! Pin used for communicating with WS2812B
    uint8_t m_pin_led;
    //! Display configuration frame
    DisplayConfig m_config;
    //! Display timer
    Timer m_display_timer;
    //! Index for digit to display
    uint8_t m_digit_index;

    void
    resetHV2155();
};

#endif