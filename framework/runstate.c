#include "runstate.h"

#include <exec/types.h>

#include <proto/exec.h>

#include "windowset.h"

static BOOL running = TRUE;

void runMainLoop(void) {
  running = TRUE;
  while(running) {
    FrameworkWindow *i;
    long signalSet = Wait(windowSetSigMask());
    for(i = windowSetFirstWindow(); i != NULL; i = i->next) {
      i->kind->handleEvents(i, signalSet);
    }
  }
}

void stopRunning(void) {
  running = FALSE;
}
