#include "runstate.h"

#include <exec/types.h>

#include <proto/exec.h>

#include "window.h"

static void cleanupDeadWindows(FrameworkWindow*);

static void cleanupDeadChildWindows(FrameworkWindow *window) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    cleanupDeadWindows(i);
    i = next;
  }
}

static void cleanupDeadWindows(FrameworkWindow *window) {
  cleanupDeadChildWindows(window);

  if(window->closed) {
    forceCloseWindow(window);
  }
}

void runMainLoop(FrameworkWindow *root) {
  BOOL running = TRUE;
  while(running) {
    long signalSet = Wait(root->treeSigMask);
    handleWindowEvents(root, signalSet);
    running = !root->closed;
    cleanupDeadWindows(root);
  }
}
