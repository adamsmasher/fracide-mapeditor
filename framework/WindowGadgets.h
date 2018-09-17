#ifndef FRAMEWORK_WINDOW_GADGETS_H
#define FRAMEWORK_WINDOW_GADGETS_H

#include <intuition/intuition.h>


typedef struct WindowGadgets_tag {
  struct Gadget *glist;
  void *data;
} WindowGadgets;

#endif
