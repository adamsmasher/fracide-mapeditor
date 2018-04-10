#include "GadgetEvents.h"

GadgetUpHandler findHandlerForGadgetUp(struct Gadget *gadget) {
  /* TODO: this won't work forever but it'll work for now */
  return gadget->UserData;
}

