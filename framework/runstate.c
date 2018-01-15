#include "runstate.h"

static BOOL running = TRUE;

BOOL isRunning(void) {
  return running;
}

void stopRunning(void) {
  running = FALSE;
}
