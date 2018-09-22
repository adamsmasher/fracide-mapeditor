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
  (CloseFunction)    NULL
};

static void mapRequesterOkClicked(FrameworkWindow *mapRequester) {
  mapRequester->selected = mapRequester->highlighted - 1;
}

static void mapRequesterCancelClicked(FrameworkWindow *mapRequester) {
  mapRequester->selected = -1;
}

static struct NewGadget mapListNewGadget = {
	MAP_LIST_LEFT,  MAP_LIST_TOP,
	MAP_REQUESTER_WIDTH  - MAP_LIST_WIDTH_DELTA,
    MAP_REQUESTER_HEIGHT - MAP_LIST_HEIGHT_DELTA,
	NULL,
	&Topaz80,
	MAP_LIST_ID,
	0,
	NULL, /* visual info filled in later */
	NULL  /* user data */
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

static struct Requester requester;

typedef struct MapRequesterData_tag {
  int selected;
  int highlighted;
} MapRequesterData;

typedef struct MapRequesterGadgets_tag {
  struct Gadget *okButton;
} MapRequesterGadgets;

static WindowGadgets *createMapRequesterGadgets(void) {
    struct Gadget *gad;
    ProjectWindowData *projectData = NULL; /* TODO: fix me */
    int height = mapRequester->window ? mapRequester->window->intuitionWindow->Height : MAP_REQUESTER_HEIGHT;
    int width  = mapRequester->window ? mapRequester->window->intuitionWindow->Width  : MAP_REQUESTER_WIDTH;
    mapRequester->gadgets = NULL;

    gad = CreateContext(&mapRequester->gadgets);

    mapListNewGadget.ng_Width  = width  - MAP_LIST_WIDTH_DELTA;
    mapListNewGadget.ng_Height = height - MAP_LIST_HEIGHT_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &mapListNewGadget,
        GTLV_Labels, projectDataGetMapNames(projectData),
        GTLV_ShowSelected, NULL,
        TAG_END);

    okButtonNewGadget.ng_TopEdge = height - OK_BUTTON_BOTTOM_OFFSET;
    gad = CreateGadget(BUTTON_KIND, gad, &okButtonNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    mapRequester->okButton = gad;

    cancelButtonNewGadget.ng_TopEdge  = height - CANCEL_BUTTON_BOTTOM_OFFSET;
    cancelButtonNewGadget.ng_LeftEdge = width  - CANCEL_BUTTON_RIGHT_OFFSET;
    gad = CreateGadget(BUTTON_KIND, gad, &cancelButtonNewGadget, TAG_END);

    if(!gad) {
        goto error;
    }

    return;
error:
    FreeGadgets(mapRequester->gadgets);
    mapRequester->okButton = NULL;
}

static void freeMapRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static void resizeMapRequester(MapRequester *mapRequester) {
    RemoveGList(mapRequester->window->intuitionWindow, mapRequester->gadgets, -1);
    FreeGadgets(mapRequester->gadgets);
    SetRast(mapRequester->window->intuitionWindow->RPort, 0);
    createMapRequesterGadgets(mapRequester);
    if(!mapRequester->gadgets) {
        fprintf(stderr, "resizeMapRequester: couldn't make gadgets");
        return;
    }
    AddGList(mapRequester->window->intuitionWindow, mapRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(mapRequester->window->intuitionWindow);
    RefreshGList(mapRequester->gadgets, mapRequester->window->intuitionWindow, NULL, -1);
    GT_RefreshWindow(mapRequester->window->intuitionWindow, NULL);
}

int spawnMapRequester(FrameworkWindow *parent, char *title) {
  WindowGadgets *gadgets;
  FrameworkWindow *mapRequester;
  int selected;

  if(!Request(&requester, parent->intuitionWindow)) {
    fprintf(stderr, "spawnMapRequester: couldn't start requester\n");
    goto error;
  }

  gadgets = createMapRequesterGadgets(&mapRequester);
  if(!gadgets) {
    fprintf(stderr, "spawnMapRequester: couldn't create gadgets\n");
    goto error_EndRequest;
  }

  mapRequesterWindowKind.newWindow.Title = title;

  mapRequester = openWindowOnGlobalScreen(&mapRequesterWindowKind, gadgets);
  if(!mapRequester) {
    fprintf(stderr, "spawnMapRequester: couldn't open window\n");
    goto error_FreeGadgets;
  }

  requesterLoop(&mapRequester);

  forceCloseWindow(mapRequester);

  EndRequest(&requester, parent->intuitionWindow);

  return data.selected;

error_FreeGadgets:
  freeMapRequesterGadgets(gadgets);
error_EndRequest:
  EndRequest(&requester, parent->intuitionWindow);
error:
  return 0;
}
