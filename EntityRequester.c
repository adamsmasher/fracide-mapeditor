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
#define ENTITY_LIST_HEIGHT_DELTA 26
#define ENTITY_LIST_TOP          20
#define ENTITY_LIST_LEFT         10

#define ENTITY_NAME_WIDTH_DELTA   ENTITY_LIST_WIDTH_DELTA
#define ENTITY_NAME_HEIGHT        12
#define ENTITY_NAME_BOTTOM_OFFSET 26
#define ENTITY_NAME_LEFT          ENTITY_LIST_LEFT

static void freeEntityRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static void closeEntityRequester(FrameworkWindow *entityRequester) {
  freeEntityRequesterGadgets(entityRequester->gadgets);
  free(entityRequester->data);
}

static void entityRequesterOnSelectEntity(FrameworkWindow *entityRequester, UWORD selected) {
  EntityRequesterData *data = entityRequester->data;
  EntityRequesterGadgets *gadgets = entityRequester->gadgets->data;

  data->selected = selected + 1;

  if(data->editable) {
    ProjectWindowData *projectData = entityRequester->parent->data;
    GT_SetGadgetAttrs(gadgets->entityNameGadget, entityRequester->intuitionWindow, NULL,
      GTST_String, projectDataGetEntityName(projectData, selected),
      GA_Disabled, FALSE,
      TAG_END);
  }
}

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
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    &closeEntityRequester,
  (ClickFunction)    NULL
};

static ListViewSpec entityListSpec = {
  ENTITY_LIST_LEFT,  ENTITY_LIST_TOP,
  ENTITY_REQUESTER_WIDTH - ENTITY_LIST_WIDTH_DELTA,
  ENTITY_REQUESTER_HEIGHT - ENTITY_LIST_HEIGHT_DELTA,
  (struct List*)NULL, /* TODO: labels */
  entityRequesterOnSelectEntity
};

/* TODO: max chars? */
static StringSpec entityNameSpec = {
  ENTITY_NAME_LEFT,  ENTITY_REQUESTER_HEIGHT - ENTITY_NAME_BOTTOM_OFFSET,
  ENTITY_REQUESTER_WIDTH - ENTITY_NAME_WIDTH_DELTA, ENTITY_NAME_HEIGHT,
  (char*)NULL, /* TODO: label...why is this null? */
  TEXT_ON_LEFT, /* TODO: not actually sure about this */
  DISABLED,
  entityRequesterOnNameEntry
};

static WindowGadgets *createEntityRequesterGadgets(int width, int height, Editable editable) {
  EntityRequesterGadgets *data;
  WindowGadgets *gadgets;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "createEntityRequesterGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  data = malloc(sizeof(EntityRequesterGadgets));
  if(!data) {
    fprintf(stderr, "createEntityRequesterGadgets: couldn't allocate data\n");
    goto error_freeWindowGadgets;
  }

  entityListSpec.height = height - ENTITY_LIST_HEIGHT_DELTA;
  entityListSpec.width  = width  - ENTITY_LIST_WIDTH_DELTA;

  if(editable) {
    entityNameSpec.top   = height - ENTITY_NAME_BOTTOM_OFFSET;
    entityNameSpec.width = width  - ENTITY_NAME_WIDTH_DELTA;

    gadgets->glist = buildGadgets(
      makeListViewGadget(&entityListSpec), NULL,
      makeStringGadget(&entityNameSpec), &data->entityNameGadget,
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

  gadgets->data = data;

  return gadgets;

error_freeWindowGadgets:
  free(gadgets);
error_freeEntityRequesterGadgets:
  free(data);
error:
  return NULL;

/* TODO: fix usss 

  ProjectWindowData *projectData = what;

  gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
    GTLV_ShowSelected, entityRequester->entityNameGadget,
    GTLV_Labels, projectDataGetEntityNames(projectData),
    TAG_END);
*/
}

static FrameworkWindow *newGenericEntityRequester(FrameworkWindow *parent, char *title, Editable editable) {
  EntityRequesterData *data;
  WindowGadgets *gadgets;
  FrameworkWindow *entityRequester;

  data = malloc(sizeof(EntityRequesterData));
  if(!data) {
    fprintf(stderr, "newGenericEntityRequester: couldn't allocate data\n");
    goto error;
  }

  data->editable = editable;
  data->selected = 0;

  entityRequesterWindowKind.newWindow.Title = title;

  gadgets = createEntityRequesterGadgets(ENTITY_REQUESTER_WIDTH, ENTITY_REQUESTER_HEIGHT, editable);
  if(!gadgets) {
    fprintf(stderr, "newGenericEntityRequester: couldn't create gadgets\n");
    goto error_freeData;
  }

  entityRequester = openChildWindow(parent, &entityRequesterWindowKind, gadgets);
  if(!entityRequester) {
    fprintf(stderr, "newGenericEntityRequester: couldn't open window\n");
    goto error_freeGadgets;
  }

  entityRequester->data = data;

  return entityRequester;

error_freeGadgets:
  freeEntityRequesterGadgets(gadgets);
error_freeData:
  free(data);
error:
  return NULL;
}

FrameworkWindow *newEntityNamesEditor(FrameworkWindow *parent) {
  return newGenericEntityRequester(parent, "Entity Names Editor", EDITABLE);
}

FrameworkWindow *newEntityRequester(FrameworkWindow *parent) {
  return newGenericEntityRequester(parent, "Choose Entity...", NON_EDITABLE);
}

BOOL isEntityRequester(FrameworkWindow *window) {
  return window->kind == &entityRequesterWindowKind;
}

static void resizeEntityRequester(FrameworkWindow *entityRequester) {
  EntityRequesterData *data = entityRequester->data;

  RemoveGList(entityRequester->intuitionWindow, entityRequester->gadgets->glist, -1);
  freeEntityRequesterGadgets(entityRequester->gadgets);
  SetRast(entityRequester->intuitionWindow->RPort, 0);
  entityRequester->gadgets = createEntityRequesterGadgets(entityRequester->intuitionWindow->Width, entityRequester->intuitionWindow->Height, data->editable);
  if(!entityRequester->gadgets) {
    fprintf(stderr, "resizeEntityRequester: couldn't make gadgets");
    return;
  }
  AddGList(entityRequester->intuitionWindow, entityRequester->gadgets->glist, (UWORD)~0, -1, NULL);
  RefreshWindowFrame(entityRequester->intuitionWindow);
  RefreshGList(entityRequester->gadgets->glist, entityRequester->intuitionWindow, NULL, -1);
  GT_RefreshWindow(entityRequester->intuitionWindow, NULL);
}
