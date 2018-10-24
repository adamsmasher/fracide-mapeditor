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
  int selected;
  int highlighted;
} MapRequesterData;

typedef struct MapRequesterGadgets_tag {
  struct Gadget *okButton;
} MapRequesterGadgets;

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
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL,
  (ClickFunction)    NULL,
};

static void mapRequesterOkClicked(FrameworkWindow *mapRequester) {
  MapRequesterData *data = mapRequester->data;
  data->selected = data->highlighted - 1;
}

static void mapRequesterCancelClicked(FrameworkWindow *mapRequester) {
  MapRequesterData *data = mapRequester->data;
  data->selected = -1;
}

static ListViewSpec mapListSpec = {
  MAP_LIST_LEFT,  MAP_LIST_TOP,
  MAP_REQUESTER_WIDTH  - MAP_LIST_WIDTH_DELTA,
  MAP_REQUESTER_HEIGHT - MAP_LIST_HEIGHT_DELTA,
  NULL,
  NULL,
  NULL,
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

static BOOL initMapRequesterGadgets(WindowGadgets *gadgets) {
  MapRequesterGadgets *data = malloc(sizeof(MapRequesterGadgets));
  if(!data) {
    fprintf(stderr, "initMapRequesterGadgets: couldn't allocate data\n");
    goto error;
  }

  gadgets->glist = buildGadgets(
    makeListViewGadget(&mapListSpec), NULL,
    makeButtonGadget(&okButtonSpec), &data->okButton,
    makeButtonGadget(&cancelButtonSpec), NULL,
    NULL);
  if(!gadgets->glist) {
    fprintf(stderr, "initMapRequesterGadgets: couldn't build gadgets\n");
    goto error;
  }

  gadgets->data = data;

  return TRUE;
error:
  return FALSE;
}

static void sizeGadgets(FrameworkWindow *window) {
  int height = window ? window->intuitionWindow->Height : MAP_REQUESTER_HEIGHT;
  int width  = window ? window->intuitionWindow->Width  : MAP_REQUESTER_WIDTH;

  mapListSpec.width  = width  - MAP_LIST_WIDTH_DELTA;
  mapListSpec.height = height - MAP_LIST_HEIGHT_DELTA;

  okButtonSpec.top = height - OK_BUTTON_BOTTOM_OFFSET;

  cancelButtonSpec.top  = height - CANCEL_BUTTON_BOTTOM_OFFSET;
  cancelButtonSpec.left = width  - CANCEL_BUTTON_RIGHT_OFFSET;
}

/*
    gad = CreateGadget(LISTVIEW_KIND, gad, &mapListNewGadget,
        GTLV_Labels, projectDataGetMapNames(projectData),
        GTLV_ShowSelected, NULL,
        TAG_END);

*/

static void freeMapRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
}

/* TODO: a lot of this should be pushed into the framework?? */
static void resizeMapRequester(FrameworkWindow *mapRequester) {
  RemoveGList(mapRequester->intuitionWindow, mapRequester->gadgets->glist, -1);
  freeMapRequesterGadgets(mapRequester->gadgets);
  SetRast(mapRequester->intuitionWindow->RPort, 0);
  sizeGadgets(mapRequester);
  if(!initMapRequesterGadgets(mapRequester->gadgets)) {
    fprintf(stderr, "resizeMapRequester: couldn't remake gadgets");
    return;
  }
  AddGList(mapRequester->intuitionWindow, mapRequester->gadgets->glist, (UWORD)~0, -1, NULL);
  RefreshWindowFrame(mapRequester->intuitionWindow);
  RefreshGList(mapRequester->gadgets->glist, mapRequester->intuitionWindow, NULL, -1);
  GT_RefreshWindow(mapRequester->intuitionWindow, NULL);
}

int spawnMapRequester(FrameworkWindow *parent, char *title) {
  MapRequesterData data;
  WindowGadgets gadgets;
  FrameworkWindow *mapRequester;

  if(!Request(&requester, parent->intuitionWindow)) {
    fprintf(stderr, "spawnMapRequester: couldn't start requester\n");
    goto error;
  }

  if(!initMapRequesterGadgets(&gadgets)) {
    fprintf(stderr, "spawnMapRequester: couldn't create gadgets\n");
    goto error_EndRequest;
  }

  mapRequesterWindowKind.newWindow.Title = title;

  mapRequester = openWindowOnGlobalScreen(&mapRequesterWindowKind, &gadgets);
  if(!mapRequester) {
    fprintf(stderr, "spawnMapRequester: couldn't open window\n");
    goto error_FreeGadgets;
  }

  mapRequester->data = &data;

  runMainLoop(mapRequester);

  EndRequest(&requester, parent->intuitionWindow);

  freeMapRequesterGadgets(&gadgets);

  return data.selected;

error_FreeGadgets:
  freeMapRequesterGadgets(&gadgets);
error_EndRequest:
  EndRequest(&requester, parent->intuitionWindow);
error:
  return 0;
}
