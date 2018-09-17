#ifndef MAP_EDITOR_GADGETS
#define MAP_EDITOR_GADGETS

#include <intuition/intuition.h>

#include "framework/WindowGadgets.h"

typedef struct MapEditorGadgets_tag {
  struct Gadget *glist;
  struct Gadget *tilesetNameGadget;
  struct Gadget *mapNameGadget;
  struct Gadget *songNameGadget;
  struct Gadget *leftGadget;
  struct Gadget *rightGadget;
  struct Gadget *upGadget;
  struct Gadget *downGadget;
} MapEditorGadgets;

WindowGadgets *newMapEditorGadgets(void);
void freeMapEditorGadgets(WindowGadgets*);

#endif
