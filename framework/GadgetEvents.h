#ifndef FRAMEWORK_GADGET_EVENTS_H
#define FRAMEWORK_GADGET_EVENTS_H

#include <libraries/gadtools.h>

#include "window.h"

typedef void (*GadgetUpHandler)(FrameworkWindow*);

GadgetUpHandler findHandlerForGadgetUp(struct Gadget*);

#endif
