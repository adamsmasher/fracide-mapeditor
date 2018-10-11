#ifndef ENTITY_REQUESTER_H
#define ENTITY_REQUESTER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

typedef struct EntityRequesterGadgets_tag {
  struct Gadget *entityNameGadget;
} EntityRequesterGadgets;

typedef enum Editable_tag {
  NON_EDITABLE,
  EDITABLE
} Editable;

typedef struct EntityRequesterData_tag {
  int selected;
  Editable editable;
} EntityRequesterData;

FrameworkWindow *newEntityRequester(FrameworkWindow *parent);
FrameworkWindow *newEntityNamesEditor(FrameworkWindow *parent);

BOOL isEntityNamesEditor(FrameworkWindow *window);

#endif
