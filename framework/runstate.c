#include "runstate.h"

#include <proto/exec.h>

#include "windowset.h"

static BOOL running = TRUE;

void run(RunProc runProc) {
  running = TRUE;
  while(running) {
    long signalSet = Wait(windowSetSigMask());
    runProc(signalSet);
  }
}

void stopRunning(void) {
  running = FALSE;
}
