#include "DisplayDriver.hpp"

DisplayDriver::DisplayDriver():
  m_digit_index(0)
{
  resetConfig();
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

  FastLED.addLeds<WS2812B, DISPLAY_PIN_LED, RGB>(m_led_colors, DISPLAY_DIGIT_NUM);
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

void
DisplayDriver::setDigit(const uint8_t digit, const uint8_t number, const bool enable)
{
  if (digit >= DISPLAY_DIGIT_NUM)
  {
    resetConfig();
    throw std::runtime_error("Digit out of bounds");
  }

  if (number > 9)
  {
    resetConfig();
    throw std::runtime_error("Number out of bounds");
  }

  m_config[digit].enabled = enable;
  m_config[digit].number = number;
}

void
DisplayDriver::setDisplay(const uint8_t digit_set_0, const uint8_t digit_set_1, const uint8_t digit_set_2)
{

  setDigit(0, digit_set_0 / 10);
  setDigit(1, digit_set_0 % 10);
  
  setDigit(2, digit_set_1 / 10);
  setDigit(3, digit_set_1 % 10);

  setDigit(4, digit_set_2 / 10);
  setDigit(5, digit_set_2 % 10);
}

void
DisplayDriver::setDot(const uint8_t digit, const bool state, const bool right_dot)
{
  if (digit > 5)
    throw std::runtime_error("Digit out of bounds");

  if (right_dot)
    m_config[digit].rdot = state;
  else
    m_config[digit].ldot = state;
}

void
DisplayDriver::setColor(const uint8_t digit, const CRGB::HTMLColorCode color)
{
  if (digit > 5)
    throw std::runtime_error("Digit out of bounds");

  m_config[digit].color = color;
}

void
DisplayDriver::run()
{
  if (!m_display_timer.overflow())
    return;

  if (!m_config[m_digit_index].enabled)
  {
    digitalWrite(DISPLAY_PIN_STROBE, LOW);
  }
  else
  {
    // HV5122 control sets
    uint16_t select_num = 0;
    uint8_t select_dot = 0;

    // Switch left dot
    select_dot |= m_config[m_digit_index].ldot ? 1 : 0;
    // Switch right dot
    select_dot |= m_config[m_digit_index].rdot ? 1 << 1 : 0;
    // Select number bit (enable a number from 0-9)
    // and add padding for digit bit
    select_num = 1 << (m_config[m_digit_index].number + 6);
    // Select digit bit (enable which digit 0-5 displays number)
    // Digit selection is inverted due to eletronic design
    select_num |= 1 << 5 - m_digit_index;

    // Send to HV5122
    digitalWrite(DISPLAY_PIN_STROBE, HIGH);
    digitalWrite(DISPLAY_PIN_ENABLE, LOW);
    shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, 0); // Pins not connected. Can be ignored??
    shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_dot);
    shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_num >> 8);
    shiftOut(DISPLAY_PIN_DATA, DISPLAY_PIN_CLOCK, MSBFIRST, select_num & 0xff);
    digitalWrite(DISPLAY_PIN_ENABLE, HIGH); 
  }

  // Send to LEDs
  setLEDArray();
  FastLED.show();

  // Digit turnover
  if (m_digit_index == 5)
    m_digit_index = 0;
  else
    m_digit_index++;

  m_display_timer.reset();
}
