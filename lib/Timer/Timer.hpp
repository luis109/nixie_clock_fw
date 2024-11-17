#ifndef TIMER_HPP
#define TIMER_HPP

#include <Arduino.h>

struct Timer
{
  uint32_t start;
  uint32_t top;
  typedef unsigned long (*timer_ptr)();
  timer_ptr timer;

  Timer(timer_ptr _timer)
  {
    timer = _timer;
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
    start = timer();
  }

  bool
  overflow()
  {
    return timer() - start > top;
  }
};
#endif