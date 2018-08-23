#ifndef MAP_EDITOR_GADGETS
#define MAP_EDITOR_GADGETS

#include <intuition/intuition.h>

typedef struct MapEditorGadgets_tag {
  struct Gadget *tilesetNameGadget;
  struct Gadget *mapNameGadget;
  struct Gadget *songNameGadget;
  struct Gadget *leftGadget;
  struct Gadget *rightGadget;
  struct Gadget *upGadget;
  struct Gadget *downGadget;
} MapEditorGadgets;

/* builds and returns a gadget list and fills out the pointers in the argument */
struct Gadget *initMapEditorGadgets(MapEditorGadgets*);

#endif
