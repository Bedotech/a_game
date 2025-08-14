#include "timer.h"

Timer timer_init() {
  Timer timer;
  timer.start = clock();
  timer.started = true;
  timer.last_read = timer.start;

  return timer;
}

uint timer_elapsed_seconds(Timer *t) {
  return (uint)(((clock()) - t->start) / CLOCKS_PER_SEC);
}
