#include "window.h"

#include <proto/intuition.h>

#include "windowset.h"

void closeWindow(FrameworkWindow* window) {
  removeWindowFromSet(window);
  window->kind->closeWindow(window);
  CloseWindow(window->intuitionWindow);
  /* TODO: close children */
  /* TODO: handle menu stuff */
  /* TODO: free memory */
}
