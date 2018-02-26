#include "runstate.h"

#include <exec/types.h>

#include <proto/exec.h>

#include "window.h"
#include "windowset.h"

static BOOL running = TRUE;

static void cleanupDeadChildWindows(FrameworkWindow *window) {
  FrameworkWindow *i, *next;

  i = window->children;
  while(i) {
    next = i->next;

    cleanupDeadChildWindows(i);

    if(i->closed) {
      closeWindow(i);
    }

    i = next;
  }
}

static void cleanupDeadWindows(void) {
  FrameworkWindow *i, *next;

  i = windowSetFirstWindow();
  while(i) {
    next = i->next;

    cleanupDeadChildWindows(i);

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
