#include "Gadgets.h"

#include <proto/gadtools.h>

#include <stdarg.h>
#include <stdlib.h>

struct Gadget *buildGadgets(GadgetSpec *gadgetSpec, struct Gadget **gadgetPtr, ...) {
  va_list args;

  struct Gadget *glist = NULL;
  struct Gadget *gad = CreateContext(&glist);

  va_start(args, gadgetPtr);

  do {
    gad = CreateGadgetA(gadgetSpec->kind, gad, gadgetSpec->newGadget, gadgetSpec->tags);
    *gadgetPtr = gad;
    (gadgetSpec = va_arg(args, GadgetSpec*)) 
    && (gadgetPtr = va_arg(args, struct Gadget**));
  } while(gadgetSpec);

  va_end(args);

  if(gad) {
    return glist;
  } else {
    FreeGadgets(glist);
    return NULL;
  }
}
