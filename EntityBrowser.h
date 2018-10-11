#ifndef ENTITY_BROWSER_H
#define ENTITY_BROWSER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#include "Map.h"

#define ENTITY_LABEL_LENGTH 16

typedef struct EntityBrowserGadgets_tag {
  struct Gadget *addEntityGadget;
  struct Gadget *removeEntityGadget;
  struct Gadget *entityListGadget;
  struct Gadget *rowGadget;
  struct Gadget *colGadget;
  struct Gadget *VRAMSlotGadget;
  struct Gadget *addTagGadget;
  struct Gadget *deleteTagGadget;
  struct Gadget *chooseEntityGadget;
  struct Gadget *tagListGadget;
  struct Gadget *tagAliasGadget;
  struct Gadget *tagIdGadget;
  struct Gadget *tagValueGadget;
  struct Gadget *thisEntityGadget;
} EntityBrowserGadgets;

typedef struct EntityBrowserData_tag {
  char title[32];
  int selectedEntity;
  int selectedTag;
  struct List entityLabels;
  struct Node *entityNodes;
  char (*entityStrings)[ENTITY_LABEL_LENGTH];
  struct List tagLabels;
  struct Node *tagNodes;
  char (*tagStrings)[TAG_ALIAS_LENGTH + 4];
  FrameworkWindow *entityRequester;
} EntityBrowserData;

FrameworkWindow *newEntityBrowserWithMapNum(FrameworkWindow *parent, const Map*, UWORD mapNum);

#endif
