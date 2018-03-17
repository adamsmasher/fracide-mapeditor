#ifndef FRAMEWORK_GADGETS_H
#define FRAMEWORK_GADGETS_H

#include <libraries/gadtools.h>

typedef struct GadgetSpec_tag {
  unsigned long kind;
  struct NewGadget *newGadget;
  struct TagItem *tags;
} GadgetSpec;

struct Gadget *buildGadgets(GadgetSpec*, struct Gadget**, ...);

#endif
