#include "EntityRequester.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/font.h"
#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "ProjectWindowData.h"

#define ENTITY_REQUESTER_WIDTH      200
#define ENTITY_REQUESTER_HEIGHT     336
#define ENTITY_REQUESTER_MIN_HEIGHT 48

#define ENTITY_LIST_WIDTH_DELTA  35
#define ENTITY_LIST_HEIGHT_DELTA 38
#define ENTITY_LIST_TOP          20
#define ENTITY_LIST_LEFT         10

#define ENTITY_NAME_WIDTH_DELTA   ENTITY_LIST_WIDTH_DELTA
#define ENTITY_NAME_HEIGHT        12
#define ENTITY_NAME_BOTTOM_OFFSET 20
#define ENTITY_NAME_LEFT          ENTITY_LIST_LEFT

/* TODO help i'm duplicated everywhere */
static int listItemStart(int selected) {
  if(selected < 10) {
    return 2;
  } else if(selected < 100) {
    return 3;
  } else {
    return 4;
  }
}

static void entityRequesterOnSelectEntity(FrameworkWindow *entityRequester, UWORD selected) {
  EntityRequesterData *data = entityRequester->data;
  EntityRequesterGadgets *gadgets = entityRequester->gadgets->data;

  data->selected = selected + 1;

  if(data->editable) {
    ProjectWindowData *projectData = entityRequester->parent->data;
    const char *entityName = projectDataGetEntityName(projectData, selected);

    GT_SetGadgetAttrs(gadgets->entityNameGadget, entityRequester->intuitionWindow, NULL,
      GTST_String, &entityName[listItemStart(selected)],
      GA_Disabled, FALSE,
      TAG_END);
  }
}

static void entityRequesterOnNameEntry(FrameworkWindow *entityRequester) {
  EntityRequesterData *data = entityRequester->data;
  EntityRequesterGadgets *gadgets = entityRequester->gadgets->data;
  ProjectWindowData *projectData = entityRequester->parent->data;

  UWORD selected = data->selected - 1;

  char *name = ((struct StringInfo*)gadgets->entityNameGadget->SpecialInfo)->Buffer;

  projectDataUpdateEntityName(projectData, selected, name);

  GT_RefreshWindow(entityRequester->intuitionWindow, NULL);
  refreshAllEntityBrowsers();
}

static ListViewSpec entityListSpec = {
  ENTITY_LIST_LEFT,  ENTITY_LIST_TOP,
  ENTITY_REQUESTER_WIDTH - ENTITY_LIST_WIDTH_DELTA,
  ENTITY_REQUESTER_HEIGHT - ENTITY_LIST_HEIGHT_DELTA,
  (struct List*)NULL, /* fill this out before creation */
  (struct Gadget**)NULL, /* fill this out before creation */
  entityRequesterOnSelectEntity
};

static StringSpec entityNameSpec = {
  ENTITY_NAME_LEFT,  ENTITY_REQUESTER_HEIGHT - ENTITY_NAME_BOTTOM_OFFSET,
  ENTITY_REQUESTER_WIDTH - ENTITY_NAME_WIDTH_DELTA, ENTITY_NAME_HEIGHT,
  64,
  (char*)NULL,
  TEXT_ON_LEFT,
  DISABLED,
  entityRequesterOnNameEntry
};

static WindowGadgets *createEntityRequesterGadgets(int width, int height, EntityRequesterData *data) {
  EntityRequesterGadgets *gadgetData;
  WindowGadgets *gadgets;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "createEntityRequesterGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  gadgetData = malloc(sizeof(EntityRequesterGadgets));
  if(!gadgetData) {
    fprintf(stderr, "createEntityRequesterGadgets: couldn't allocate data\n");
    goto error_freeWindowGadgets;
  }

  entityListSpec.height = height - ENTITY_LIST_HEIGHT_DELTA;
  entityListSpec.width  = width  - ENTITY_LIST_WIDTH_DELTA;
  entityListSpec.labels = data->entityNames;

  if(data->editable) {
    entityNameSpec.top   = height - ENTITY_NAME_BOTTOM_OFFSET;
    entityNameSpec.width = width  - ENTITY_NAME_WIDTH_DELTA;

    gadgets->glist = buildGadgets(
      makeStringGadget(&entityNameSpec), &gadgetData->entityNameGadget,
      makeListViewGadget(&entityListSpec), NULL,
      NULL);
  } else {
    gadgets->glist = buildGadgets(
      makeListViewGadget(&entityListSpec), NULL,
      NULL);
  }
  if(!gadgets->glist) {
    fprintf(stderr, "createEntityRequesterGadgets: failed to create gadgets\n");
    goto error_freeEntityRequesterGadgets;
  }

  gadgets->data = gadgetData;

  return gadgets;

error_freeWindowGadgets:
  free(gadgets);
error_freeEntityRequesterGadgets:
  free(gadgetData);
error:
  return NULL;
}

static void freeEntityRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static void closeEntityRequester(FrameworkWindow *entityRequester) {
  free(entityRequester->data);
}

static WindowKind entityRequesterWindowKind = {
  {
    40, 40, ENTITY_REQUESTER_WIDTH, ENTITY_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Entity Editor",
    NULL,
    NULL,
    ENTITY_REQUESTER_WIDTH, ENTITY_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (GadgetBuilder)    createEntityRequesterGadgets,
  (GadgetFreer)      freeEntityRequesterGadgets,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    closeEntityRequester,
  (ClickFunction)    NULL
};

static FrameworkWindow *newGenericEntityRequester(FrameworkWindow *parent, char *title, struct List *entityNames, Editable editable) {
  EntityRequesterData *data;
  FrameworkWindow *entityRequester;

  data = malloc(sizeof(EntityRequesterData));
  if(!data) {
    fprintf(stderr, "newGenericEntityRequester: couldn't allocate data\n");
    goto error;
  }

  data->editable = editable;
  data->selected = 0;
  data->entityNames = entityNames;

  entityRequesterWindowKind.newWindow.Title = title;

  entityRequester = openChildWindow(parent, &entityRequesterWindowKind, data);
  if(!entityRequester) {
    fprintf(stderr, "newGenericEntityRequester: couldn't open window\n");
    goto error_freeData;
  }

  return entityRequester;

error_freeData:
  free(data);
error:
  return NULL;
}

FrameworkWindow *newEntityNamesEditor(FrameworkWindow *parent, struct List *entityNames) {
  return newGenericEntityRequester(parent, "Entity Names Editor", entityNames, EDITABLE);
}

FrameworkWindow *newEntityRequester(FrameworkWindow *parent, struct List *entityNames) {
  return newGenericEntityRequester(parent, "Choose Entity...", entityNames, NON_EDITABLE);
}

static BOOL isEntityRequester(FrameworkWindow *window) {
  return (BOOL)(window->kind == &entityRequesterWindowKind);
}

BOOL isEntityNamesEditor(FrameworkWindow *window) {
  if(isEntityRequester(window)) {
    EntityRequesterData *data = window->data;
    return (BOOL)data->editable;
  }
  return FALSE;
}
