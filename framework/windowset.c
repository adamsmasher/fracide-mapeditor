#include "windowset.h"

static long sigMask = 0;
static FrameworkWindow *firstWindow = NULL;

FrameworkWindow *windowSetFirstWindow(void) {
  return firstWindow;
}

void addWindowToSet(FrameworkWindow *window) {
  if(firstWindow) {
    FrameworkWindow *i = firstWindow;
    while(i->next) {
      i = i->next;
    }
    i->next = window;
    window->prev = i;
  } else {
    firstWindow = window;
  }

  sigMask |= 1L << window->intuitionWindow->UserPort->mp_SigBit;
}

void removeWindowFromSet(FrameworkWindow *window) {
  if(firstWindow == window) {
    firstWindow = NULL;
  }

  if(window->prev) {
    window->prev->next = window->next;
  }

  if(window->next) {
    window->next->prev = window->prev;
  }

  sigMask &= ~(1L << window->intuitionWindow->UserPort->mp_SigBit);
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
