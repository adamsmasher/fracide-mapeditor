#include "window.h"

#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdlib.h>

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
  /* TODO: do menu stuff */
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
  window->kind->closeWindow(window);

  ClearMenuStrip(window->intuitionWindow);
  FreeMenus(window->menu);

  removeWindowFromSet(window);
  CloseWindow(window->intuitionWindow);

  free(window);
}
