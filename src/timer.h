#ifndef TIMER_H
#define TIMER_H

#include "common.h"
#include <time.h>


typedef struct {
  uint tick_every;
  clock_t start;
  clock_t last_read;
  bool started;
  long number_of_tick;
} Timer;

// Initialize the timer
void timer_init(Timer *t);
void timer_start(Timer *t);
void timer_stop(Timer *t);
bool timer_tick(Timer *t);
#endif
