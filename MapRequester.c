#include "MapRequester.h"

#include <stdio.h>
#include <stdlib.h>

#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include "framework/font.h"
#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/runstate.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "NumberedList.h"
#include "Project.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"

#define MAP_REQUESTER_WIDTH      240
#define MAP_REQUESTER_HEIGHT     336
#define MAP_REQUESTER_MIN_HEIGHT 100

#define MAP_LIST_LEFT         20
#define MAP_LIST_TOP          20
#define MAP_LIST_WIDTH_DELTA  52
#define MAP_LIST_HEIGHT_DELTA 72

#define OK_BUTTON_LEFT          20
#define OK_BUTTON_BOTTOM_OFFSET 36
#define OK_BUTTON_WIDTH         72
#define OK_BUTTON_HEIGHT        16

#define CANCEL_BUTTON_RIGHT_OFFSET  104
#define CANCEL_BUTTON_BOTTOM_OFFSET 36
#define CANCEL_BUTTON_WIDTH         72
#define CANCEL_BUTTON_HEIGHT        16

static struct Requester requester;

typedef struct MapRequesterData_tag {
  UWORD selected;
  UWORD highlighted;
  struct List *mapNames;
} MapRequesterData;

typedef struct MapRequesterGadgets_tag {
  struct Gadget *okButton;
} MapRequesterGadgets;

static void mapRequesterOkClicked(FrameworkWindow *mapRequester) {
  MapRequesterData *data = mapRequester->data;
  data->selected = data->highlighted;
  mapRequester->closed = TRUE;
}

static void mapRequesterCancelClicked(FrameworkWindow *mapRequester) {
  MapRequesterData *data = mapRequester->data;
  data->selected = 0;
  mapRequester->closed = TRUE;
}

static void mapRequesterOnSelect(FrameworkWindow *mapRequester, UWORD selected) {
  MapRequesterData *data = mapRequester->data;
  MapRequesterGadgets *gadgets = mapRequester->gadgets->data;

  data->highlighted = selected + 1;
  GT_SetGadgetAttrs(gadgets->okButton, mapRequester->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    TAG_END);
}

static ListViewSpec mapListSpec = {
  MAP_LIST_LEFT,  MAP_LIST_TOP,
  MAP_REQUESTER_WIDTH  - MAP_LIST_WIDTH_DELTA,
  MAP_REQUESTER_HEIGHT - MAP_LIST_HEIGHT_DELTA,
  NULL,
  NULL,
  mapRequesterOnSelect,
};

static ButtonSpec okButtonSpec = {
  OK_BUTTON_LEFT, MAP_REQUESTER_HEIGHT - OK_BUTTON_BOTTOM_OFFSET,
  OK_BUTTON_WIDTH, OK_BUTTON_HEIGHT,
  "OK",
  TEXT_INSIDE,
  DISABLED,
  mapRequesterOkClicked
};

static ButtonSpec cancelButtonSpec = {
  MAP_REQUESTER_WIDTH  - CANCEL_BUTTON_RIGHT_OFFSET,
  MAP_REQUESTER_HEIGHT - CANCEL_BUTTON_BOTTOM_OFFSET,
  CANCEL_BUTTON_WIDTH, CANCEL_BUTTON_HEIGHT,
  "Cancel",
  TEXT_INSIDE,
  ENABLED,
  mapRequesterCancelClicked
};

static const char *safeGetMapName(ProjectWindowData *data, UWORD mapNum) {
  const char *mapName = projectDataGetMapName(data, mapNum);
  if(mapName) {
    return mapName;
  } else {
    return "";
  }
}

static WindowGadgets *createMapRequesterGadgets(WORD width, WORD height, void *data) {
  MapRequesterGadgets *gadgetData;
  WindowGadgets *gadgets;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "createMapRequesterGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  gadgetData = malloc(sizeof(MapRequesterGadgets));
  if(!gadgetData) {
    fprintf(stderr, "createMapRequesterGadgets: couldn't allocate data\n");
    goto error_freeWindowGadgets;
  }

  mapListSpec.width  = width  - MAP_LIST_WIDTH_DELTA;
  mapListSpec.height = height - MAP_LIST_HEIGHT_DELTA;
  mapListSpec.labels = ((MapRequesterData*)data)->mapNames;

  okButtonSpec.top = height - OK_BUTTON_BOTTOM_OFFSET;

  cancelButtonSpec.top  = height - CANCEL_BUTTON_BOTTOM_OFFSET;
  cancelButtonSpec.left = width  - CANCEL_BUTTON_RIGHT_OFFSET;

  gadgets->glist = buildGadgets(
    makeListViewGadget(&mapListSpec), NULL,
    makeButtonGadget(&okButtonSpec), &gadgetData->okButton,
    makeButtonGadget(&cancelButtonSpec), NULL,
    NULL);
  if(!gadgets->glist) {
    fprintf(stderr, "createMapRequesterGadgets: couldn't build gadgets\n");
    goto error_freeMapRequesterGadgets;
  }

  gadgets->data = gadgetData;

  return gadgets;

error_freeMapRequesterGadgets:
  free(gadgetData);
error_freeWindowGadgets:
  free(gadgets);
error:
  return NULL;
}

static void freeMapRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static WindowKind mapRequesterWindowKind = {
  {
    40, 40, MAP_REQUESTER_WIDTH, MAP_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Save Map",
    NULL,
    NULL,
    MAP_REQUESTER_WIDTH, MAP_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (GadgetBuilder)    createMapRequesterGadgets,
  (GadgetFreer)      freeMapRequesterGadgets,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL,
  (ClickFunction)    NULL,
};

UWORD spawnMapRequester(FrameworkWindow *parent, char *title) {
  struct List *mapNameList;
  MapRequesterData data;
  FrameworkWindow *mapRequester;

  if(!Request(&requester, parent->intuitionWindow)) {
    fprintf(stderr, "spawnMapRequester: couldn't start requester\n");
    goto error;
  }

  mapNameList = newNumberedList(safeGetMapName, parent->data, MAX_MAPS_IN_PROJECT);
  if(!mapNameList) {
    fprintf(stderr, "spawnMapRequester: couldn't make map name list\n");
    goto error_EndRequest;
  }

  mapRequesterWindowKind.newWindow.Title = title;

  data.selected = 0;
  data.mapNames = mapNameList;

  mapRequester = openChildWindow(parent, &mapRequesterWindowKind, &data);
  if(!mapRequester) {
    fprintf(stderr, "spawnMapRequester: couldn't open window\n");
    goto error_freeNameList;
  }

  runMainLoop(mapRequester);

  EndRequest(&requester, parent->intuitionWindow);

  freeNumberedList(mapNameList);

  return data.selected;

error_freeNameList:
  freeNumberedList(mapNameList);
error_EndRequest:
  EndRequest(&requester, parent->intuitionWindow);
error:
  return 0;
}
