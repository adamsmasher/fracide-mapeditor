#include "windowset.h"

static long sigMask = 0;
static FrameworkWindow *firstWindow = NULL;

FrameworkWindow *windowSetFirstWindow(void) {
  return firstWindow;
}

void addWindowToSet(FrameworkWindow *window) {
  FrameworkWindow **i = &firstWindow;
  while(*i) {
    i = &(*i)->next;
  }
  *i = window;

  sigMask |= 1L << window->intuitionWindow->UserPort->mp_SigBit;
}

void removeWindowFromSet(FrameworkWindow *window) {
  sigMask &= ~(1L << window->intuitionWindow->UserPort->mp_SigBit);
  /* TODO: actually remove the window from the set! */
}

void closeAllWindows(void) {
  FrameworkWindow *i = firstWindow;
  while(i) {
    FrameworkWindow *next = i->next;
    closeWindow(i);
    i = next;
  }
  firstWindow = NULL;
}

long windowSetSigMask(void) {
  return sigMask;
}
