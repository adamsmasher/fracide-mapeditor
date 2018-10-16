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
  struct List *entityNames;
} EntityRequesterData;

FrameworkWindow *newEntityRequester(FrameworkWindow *parent, struct List *entityNames);
FrameworkWindow *newEntityNamesEditor(FrameworkWindow *parent, struct List *entityNames);

BOOL isEntityNamesEditor(FrameworkWindow *window);

#endif
