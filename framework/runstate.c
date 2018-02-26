#include "runstate.h"

#include <exec/types.h>

#include <proto/exec.h>

#include "window.h"
#include "windowset.h"

static BOOL running = TRUE;

static void cleanupDeadWindows(void) {
  FrameworkWindow *i, *next;

  i = windowSetFirstWindow();
  while(i) {
    /* TODO: handle child windows */
    next = i->next;
    if(i->closed) {
      closeWindow(i);
    }
    i = next;
  }
}

void runMainLoop(void) {
  running = TRUE;
  while(running) {
    FrameworkWindow *i;
    long signalSet = Wait(windowSetSigMask());
    for(i = windowSetFirstWindow(); i != NULL; i = i->next) {
      /* TODO: handle child windows */
      handleWindowEvents(i, signalSet);
    }
    cleanupDeadWindows();
  }
}

void stopRunning(void) {
  running = FALSE;
}
