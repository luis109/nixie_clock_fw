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

  // Use the fastled library instead
  // pinMode(pin_led, OUTPUT);
  // digitalWrite(pin_led, LOW);

  m_display_timer.setTop(1000);
}

bool
DisplayDriver::setDisplay(const uint8_t digit_set_0, const uint8_t digit_set_1, const uint8_t digit_set_2)
{
  if (digit_set_0 > 99 || digit_set_1 > 99 || digit_set_2 > 99)
  {
    for (uint8_t i = 0; i < 6; i++)
      m_config.digit[i] = 0;
    
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

// void
// DisplayDriver::run()
// {
//   // HV5122 control
//   uint16_t select_num;
//   uint8_t select_digit;

//   digitalWrite(pin_enabled, LOW);
//   digitalWrite(pin_clk, HIGH);

//   for (uint8_t i = 0; i < 6; i++)
//   {
//     select_num = 1 << (m_config.digit[i] + 6);
//     select_digit = 1 << i;
//     select_num |= select_digit;

//     digitalWrite(pin_enabled, LOW);
//     shiftOut(pin_data, pin_clk, MSBFIRST, select_num >> 8);
//     shiftOut(pin_data, pin_clk, MSBFIRST, select_num & 0xff);
//     digitalWrite(pin_enabled, HIGH);
//     delay(1);
//   }
// }

void
DisplayDriver::run()
{
  if (!m_display_timer.overflow())
    return;

  // HV5122 control sets
  uint16_t select_num;
  uint8_t select_digit;

  // Select number bit (enable a number from 0-9)
  select_num = 1 << (m_config.digit[m_digit_index] + 6);
  // Select digit bit (enable which digit 0-5 displays number)
  // Digit selection is inverted due to eletronic design
  select_digit = 1 << 5 - m_digit_index;
  // Join selection
  select_num |= select_digit;

  // Send to HV5122
  digitalWrite(m_pin_enabled, LOW);
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
