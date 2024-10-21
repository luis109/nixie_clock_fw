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

    //! Set LED color
    //! @param color digit to display dot (0 - 5)
    //! @return true dot is set, false otherwise
    void
    setColor(const CRGB::HTMLColorCode color);

    //! Main display 
    void
    run();

  private:

    //! Display configuration frame
    struct DisplayConfig
    {
      // Digits
      uint8_t digit[6];
      // Right dots
      bool rdot[6];
      // Left dots
      bool ldot[6];
      // LEDs
      CRGB leds[DISPLAY_DIGIT_NUM];

      DisplayConfig()
      {
        reset();
      }

      void
      reset()
      {
        for (uint8_t i = 0; i < DISPLAY_DIGIT_NUM; i++)
        {
          digit[i] = 0;
          rdot[i] = false;
          ldot[i] = false;
          leds[i] = CRGB::Black;
        }
      }

      void
      resetDots()
      {
        for (uint8_t i = 0; i < DISPLAY_DIGIT_NUM; i++)
        {
          rdot[i] = false;
          ldot[i] = false;
        }
      }

      void
      resetLEDs()
      {
        for (uint8_t i = 0; i < DISPLAY_DIGIT_NUM; i++)
        {
          leds[i] = CRGB::Black;
        }
      }
    } m_config;
    //! Display timer
    Timer m_display_timer;
    //! Index for digit to display
    uint8_t m_digit_index;

    void
    resetHV2155();
};

#endif