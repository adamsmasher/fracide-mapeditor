#include "window.h"

#include <proto/intuition.h>

#include "windowset.h"

static void handleWindowChildEvents(FrameworkWindow *window, long signalSet) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    handleWindowEvents(i, signalSet);
    i = next;
  }
}

void handleWindowEvents(FrameworkWindow *window, long signalSet) {
  window->kind->handleEvents(window, signalSet);
  handleWindowChildEvents(window, signalSet);
}

static void closeChildren(FrameworkWindow *window) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    closeWindow(i);
    i = next;
  }
}

void closeWindow(FrameworkWindow *window) {
  closeChildren(window);
  removeWindowFromSet(window);
  window->kind->closeWindow(window);
  CloseWindow(window->intuitionWindow);
  /* TODO: handle menu stuff */
  /* TODO: free memory */
}
