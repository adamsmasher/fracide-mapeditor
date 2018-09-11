#ifndef MAP_EDITOR_GADGETS
#define MAP_EDITOR_GADGETS

#include <intuition/intuition.h>

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

/* fills out the pointers in the argument; returns TRUE on success */
BOOL initMapEditorGadgets(MapEditorGadgets*);

#endif
