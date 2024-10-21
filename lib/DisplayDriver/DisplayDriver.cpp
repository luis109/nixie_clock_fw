#include "DisplayDriver.hpp"

DisplayDriver::DisplayDriver():
  m_digit_index(0)
{
  m_config.reset();
  m_display_timer.reset();
}

void
DisplayDriver::begin()
{
  // Set pin modes
  pinMode(DISPLAY_PIN_ENABLE, OUTPUT);
  digitalWrite(DISPLAY_PIN_ENABLE, LOW);

  pinMode(DISPLAY_PIN_STROBE, OUTPUT);
  digitalWrite(DISPLAY_PIN_STROBE, HIGH);

  pinMode(DISPLAY_PIN_CLOCK, OUTPUT);
  digitalWrite(DISPLAY_PIN_CLOCK, LOW);

  pinMode(DISPLAY_PIN_DATA, OUTPUT);
  digitalWrite(DISPLAY_PIN_DATA, LOW);

  resetHV2155();

  FastLED.addLeds<WS2812B, DISPLAY_PIN_LED, RGB>(m_config.leds, DISPLAY_DIGIT_NUM);
  // Use the fastled library instead
  // pinMode(pin_led, OUTPUT);
  // digitalWrite(pin_led, LOW);

  m_display_timer.setTop(1000);
}

void
DisplayDriver::resetHV2155()
{
  // Set HV2155 shifters to zero
  digitalWrite(DISPLAY_PIN_ENABLE, LOW);
  for (uint8_t i = 0; i < 4; i++)
    shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, 0);
  digitalWrite(DISPLAY_PIN_ENABLE, HIGH);
}

bool
DisplayDriver::setDisplay(const uint8_t digit_set_0, const uint8_t digit_set_1, const uint8_t digit_set_2)
{
  if (digit_set_0 > 99 || digit_set_1 > 99 || digit_set_2 > 99)
  {
    m_config.reset();
    return false;
  }

  m_config.digit[0] = digit_set_0 / 10;
  m_config.digit[1] = digit_set_0 % 10;
  
  m_config.digit[2] = digit_set_1 / 10;
  m_config.digit[3] = digit_set_1 % 10;

  m_config.digit[4] = digit_set_2 / 10;
  m_config.digit[5] = digit_set_2 % 10;

  return true;
}

bool
DisplayDriver::setDot(const uint8_t digit)
{
  m_config.resetDots();
  if (digit > 5)
    return false;
  
  m_config.ldot[digit] = true;

  return true;
}

void
DisplayDriver::setColor(const CRGB::HTMLColorCode color)
{
  for (uint8_t i = 0; i < DISPLAY_DIGIT_NUM; i++)
  {
    m_config.leds[i] = color;
  }
}

void
DisplayDriver::run()
{
  if (!m_display_timer.overflow())
    return;

  // HV5122 control sets
  uint16_t select_num = 0;
  uint8_t select_dot = 0;

  // Switch left dot
  select_dot |= m_config.ldot[m_digit_index] ? 1 : 0;
  // Switch right dot
  select_dot |= m_config.rdot[m_digit_index] ? 1 << 1 : 0;
  // Select number bit (enable a number from 0-9)
  // and add padding for digit bit
  select_num = 1 << (m_config.digit[m_digit_index] + 6);
  // Select digit bit (enable which digit 0-5 displays number)
  // Digit selection is inverted due to eletronic design
  select_num |= 1 << 5 - m_digit_index;

  // Send to HV5122
  digitalWrite(DISPLAY_PIN_ENABLE, LOW);
  shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, 0); // Can be ignored
  shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_dot);
  shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_num >> 8);
  shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_num & 0xff);
  digitalWrite(DISPLAY_PIN_ENABLE, HIGH);

  // Send to LEDs
  FastLED.show();

  // Digit turnover
  if (m_digit_index == 5)
    m_digit_index = 0;
  else
    m_digit_index++;

  m_display_timer.reset();
}
