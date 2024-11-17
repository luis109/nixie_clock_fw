#ifndef DISPLAY_DRIVER_HPP
#define DISPLAY_DRIVER_HPP

#include <Arduino.h>
#include <FastLED.h>
#include "Timer.hpp"
#include "DisplayDriverDefinitions.hpp"

class DisplayDriver
{
  public:
    DisplayDriver();
    ~DisplayDriver();

    //! Initialize the display
    void
    begin();

    //! Set individual display digit
    //! @param digit digit to set. Must be < 6
    //! @param number number to display. Must be < 10
    //! @throw Exception if digit or number are out of bounds
    void
    setDigit(const uint8_t digit, const uint8_t number, const bool enable = true);

    //! Set display digits, by digit set
    //! @param digit_set_0 first digit set (leftmost 2 digits)
    //! @param digit_set_1 second digit set (center 2 digits)
    //! @param digit_set_2 third digit set (rightmost 2 digits)
    //! @throw Exception if numbers are out of bounds (digit_set > 99)
    void
    setDisplay(const uint8_t digit_set_0, const uint8_t digit_set_1, const uint8_t digit_set_2);

    //! Set dot to display
    //! @param digit digit to display dot (0 - 5)
    //! @param state state of dot. True to turn on, false otherwise.
    //! @param right_dot true to turn on right dot, false for left dot.
    void
    setDot(const uint8_t digit, const bool state, const bool right_dot = true);

    //! Set LED color
    //! @param digit digit to set color (0 - 5)
    void
    setColor(const uint8_t digit, const CRGB::HTMLColorCode color);

    //! Main display 
    void
    run();

  private:
    //! Display configuration frame
    struct DisplayDigit
    {
      // Enable number (true to turn digit on, false otherwise)
      bool enabled;
      // Number to display
      uint8_t number;
      // Display right dot
      bool rdot;
      // Display left dot
      bool ldot;
      // LED color
      CRGB color;

      DisplayDigit()
      {
        reset();
      }

      void
      reset()
      {
        enabled = true;
        number = 0;
        rdot = false;
        ldot = false;
        color = CRGB::Black;
      }
    } m_config[DISPLAY_DIGIT_NUM];
    //! Display timer
    Timer* m_display_timer;
    //! Index for digit to display
    uint8_t m_digit_index;
    // LED color
    CRGB m_led_colors[DISPLAY_DIGIT_NUM];

    void
    resetHV2155();

    void
    resetConfig()
    {
      for (uint8_t i = 0; i<DISPLAY_DIGIT_NUM; i++)
        m_config[i].reset();
    }

    void
    resetLEDs()
    {
      for (uint8_t i = 0; i<DISPLAY_DIGIT_NUM; i++)
        m_config[i].color = CRGB::Black;
    }

    void
    setLEDArray()
    {
      for (uint8_t i = 0; i<DISPLAY_DIGIT_NUM; i++)
        m_led_colors[i] = m_config[i].color;
    }
};

#endif