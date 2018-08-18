#include "TilesetRequester.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/font.h"
#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "MapEditor.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"
#include "TilesetPackage.h"

#define TILESET_REQUESTER_WIDTH      200
#define TILESET_REQUESTER_HEIGHT     336
#define TILESET_REQUESTER_MIN_HEIGHT 48

#define TILESET_LIST_WIDTH_DELTA  35
#define TILESET_LIST_HEIGHT_DELTA 26
#define TILESET_LIST_TOP          20
#define TILESET_LIST_LEFT         10

static WindowKind tilesetRequesterWindowKind = {
  {
    40, 40, TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Choose Tileset",
    NULL,
    NULL,
    TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL
};

BOOL isTilesetRequesterWindow(FrameworkWindow *window) {
  return (BOOL)(window->kind == &tilesetRequesterWindowKind);
}

static void handleTilesetRequesterGadgetUp(FrameworkWindow *mapEditorWindow, struct IntuiMessage *msg) {
  mapEditorSetTileset(mapEditorWindow, msg->Code);
}

static struct NewGadget tilesetListNewGadget = {
    TILESET_LIST_LEFT,  TILESET_LIST_TOP,
    TILESET_REQUESTER_WIDTH  - TILESET_LIST_WIDTH_DELTA,
    TILESET_REQUESTER_HEIGHT - TILESET_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    TILESET_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static void initTilesetRequesterVi(void) {
  void *vi = getGlobalVi();
  if(!vi) {
    fprintf(stderr, "initTilesetRequesterVi: failed to get global VI");
  }

  tilesetListNewGadget.ng_VisualInfo = vi;
}

static void createTilesetRequesterGadgets(TilesetRequesterData *data, FrameworkWindow *window) {
  struct Gadget *gad;
  struct Gadget *glist = NULL;
  int height = window ? window->intuitionWindow->Height : TILESET_REQUESTER_HEIGHT;
  int width  = window ? window->intuitionWindow->Width  : TILESET_REQUESTER_WIDTH;

  gad = CreateContext(&glist);

  tilesetListNewGadget.ng_Height = height - TILESET_LIST_HEIGHT_DELTA;
  tilesetListNewGadget.ng_Width  = width  - TILESET_LIST_WIDTH_DELTA;
    /* TODO: FIX ME */
/*    gad = CreateGadget(LISTVIEW_KIND, gad, &tilesetListNewGadget,
        GTLV_Labels, &tilesetPackage->tilesetNames,
        TAG_END); */
  data->tilesetList = gad;

  if(gad) {
    data->gadgets = glist;
  } else {
    data->tilesetList = NULL;
    FreeGadgets(glist);
    data->gadgets = NULL;
  }	
}

FrameworkWindow *newTilesetRequester(char *title, FrameworkWindow *parent) {
  FrameworkWindow *window;
  TilesetRequesterData *data = malloc(sizeof(TilesetRequesterData));
  if(!data) {
    fprintf(stderr, "newTilesetRequester: failed to allocate data\n");
    goto error;
  }

  data->title = malloc(strlen(title) + 1);
  if(!data->title) {
    fprintf(stderr, "newTilesetRequester: couldn't allocate title\n");
    goto error_freeData;
  }
  strcpy(data->title, title);

  initTilesetRequesterVi();
  createTilesetRequesterGadgets(data, NULL);
  if(!data->gadgets) {
    goto error_freeTitle;
  }
  tilesetRequesterWindowKind.newWindow.FirstGadget = data->gadgets;

  tilesetRequesterWindowKind.newWindow.Title = data->title;
  window = openChildWindow(parent, &tilesetRequesterWindowKind);
  if(!window) {
    goto error_freeGadgets;
  }
  window->data = data;

  data->closed = 0;

  return window;
error_freeGadgets:
  FreeGadgets(data->gadgets);
error_freeTitle:
  free(data->title);
error_freeData:
  free(data);
error:
  return NULL;
}

void closeTilesetRequester(FrameworkWindow *tilesetRequester) {
  TilesetRequesterData *data = tilesetRequester->data;
  /* TODO: the framework should free the gadgets */
  FreeGadgets(data->gadgets);
  free(data->title);
  free(data);
}

void refreshTilesetRequesterList(FrameworkWindow *tilesetRequester) {
  TilesetRequesterData *data = tilesetRequester->data;
  ProjectWindowData *parentData = tilesetRequester->parent->data;
  TilesetPackage *tilesetPackage = NULL; /* TODO: fix me parentData->tilesetPackage;*/

  GT_SetGadgetAttrs(data->tilesetList, tilesetRequester->intuitionWindow, NULL,
    GTLV_Labels, &tilesetPackage->tilesetNames,
    TAG_END);
}

void resizeTilesetRequester(FrameworkWindow *tilesetRequester) {
  TilesetRequesterData *data = tilesetRequester->data;
  RemoveGList(tilesetRequester->intuitionWindow, data->gadgets, -1);
  FreeGadgets(data->gadgets);
  SetRast(tilesetRequester->intuitionWindow->RPort, 0);
  createTilesetRequesterGadgets(data, tilesetRequester);
  if(!data->gadgets) {
    fprintf(stderr, "resizeTilesetRequester: couldn't make gadgets");
    return;
  }
  AddGList(tilesetRequester->intuitionWindow, data->gadgets, (UWORD)~0, -1, NULL);
  RefreshWindowFrame(tilesetRequester->intuitionWindow);
  RefreshGList(data->gadgets, tilesetRequester->intuitionWindow, NULL, -1);
  GT_RefreshWindow(tilesetRequester->intuitionWindow, NULL);
}
