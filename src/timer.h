#ifndef TIMER_H
#define TIMER_H

#include "common.h"
#include <time.h>


typedef struct {
  unsigned int tick_every;
  clock_t start;
  clock_t last_read;
  bool started;
  long number_of_tick;
} Timer;

// Initialize the timer
Timer timer_init(void);
unsigned int timer_elapsed_seconds(Timer *t);
void timer_start(Timer *t);
void timer_stop(Timer *t);
bool timer_tick(Timer *t);
#endif
