#include "timer.h"

Timer timer_init() {
  Timer timer;
  timer.start = clock();
  timer.started = true;
  timer.last_read = timer.start;

  return timer;
}

unsigned int timer_elapsed_seconds(Timer *t) {
  return (unsigned int)(((clock()) - t->start) / CLOCKS_PER_SEC);
}
