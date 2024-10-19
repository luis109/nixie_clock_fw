#include "DisplayDriver.hpp"

DisplayDriver::DisplayDriver(uint8_t pin_enabled, uint8_t pin_str, uint8_t pin_clk, uint8_t pin_data, uint8_t pin_led):
  m_pin_enabled(pin_enabled),
  m_pin_str(pin_str),
  m_pin_clk(pin_clk),
  m_pin_data(pin_data),
  m_pin_led(pin_led),
  m_digit_index(0)
{
  m_config.reset();
  m_display_timer.reset();
}

void
DisplayDriver::begin()
{
  // Set pin modes
  pinMode(m_pin_enabled, OUTPUT);
  digitalWrite(m_pin_enabled, LOW);

  pinMode(m_pin_str, OUTPUT);
  digitalWrite(m_pin_str, HIGH);

  pinMode(m_pin_clk, OUTPUT);
  digitalWrite(m_pin_clk, LOW);

  pinMode(m_pin_data, OUTPUT);
  digitalWrite(m_pin_data, LOW);

  resetHV2155();

  // Use the fastled library instead
  // pinMode(pin_led, OUTPUT);
  // digitalWrite(pin_led, LOW);

  m_display_timer.setTop(1000);
}

void
DisplayDriver::resetHV2155()
{
  // Set HV2155 shifters to zero
  digitalWrite(m_pin_enabled, LOW);
  for (uint8_t i = 0; i < 4; i++)
    shiftOut(m_pin_data, m_pin_clk, MSBFIRST, 0);
  digitalWrite(m_pin_enabled, HIGH);
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
DisplayDriver::run()
{
  if (!m_display_timer.overflow())
    return;

  // HV5122 control sets
  uint16_t select_num = 0;
  uint8_t select_dot = 0;

  // Switch left dot
  if (m_config.ldot[m_digit_index])
    select_dot |= 1 << 0;
  // Switch right dot
  if (m_config.rdot[m_digit_index])
    select_dot |= 1 << 1;
  // Select number bit (enable a number from 0-9)
  // and add padding for digit bit
  select_num = 1 << (m_config.digit[m_digit_index] + 6);
  // Select digit bit (enable which digit 0-5 displays number)
  // Digit selection is inverted due to eletronic design
  select_num |= 1 << 5 - m_digit_index;

  // Send to HV5122
  digitalWrite(m_pin_enabled, LOW);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, 0); // Can be ignored
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_dot);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_num >> 8);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_num & 0xff);
  digitalWrite(m_pin_enabled, HIGH);

  // Digit turnover
  if (m_digit_index == 5)
    m_digit_index = 0;
  else
    m_digit_index++;

  m_display_timer.reset();
}

void
DisplayDriver::test()
{
  m_digit_index = 0;

  // HV5122 control sets
  uint16_t select_num = 0;
  uint8_t select_dot = 0;

  // Switch left dot
  if (m_config.ldot[m_digit_index])
    select_dot |= 1 << 0;
  // Switch right dot
  if (m_config.rdot[m_digit_index])
    select_dot |= 1 << 1;
  // Select number bit (enable a number from 0-9)
  // and add padding for digit bit
  select_num = 1 << (m_config.digit[m_digit_index] + 6);
  // Select digit bit (enable which digit 0-5 displays number)
  // Digit selection is inverted due to eletronic design
  select_num |= 1 << 5 - m_digit_index;

  // Send to HV5122
  digitalWrite(m_pin_enabled, LOW);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, 0); // Can be ignored
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_dot);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_num >> 8);
  shiftOut(m_pin_data, m_pin_clk, MSBFIRST, select_num & 0xff);
  digitalWrite(m_pin_enabled, HIGH);
}
