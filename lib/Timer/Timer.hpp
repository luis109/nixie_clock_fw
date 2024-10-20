#ifndef TIMER_HPP
#define TIMER_HPP

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
#endif