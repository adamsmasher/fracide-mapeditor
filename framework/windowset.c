#include "windowset.h"

static long sigMask = 0;
static FrameworkWindow *firstWindow = NULL;

FrameworkWindow *windowSetFirstWindow(void) {
  return firstWindow;
}

/* TODO: write an openWindow function that automatically does this */
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