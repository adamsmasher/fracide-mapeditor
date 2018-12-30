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
#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "MapEditorData.h"
#include "NumberedList.h"
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

typedef struct TilesetRequesterData_tag {
  char *title;
  struct List *tilesetNames;
} TilesetRequesterData;

typedef struct TilesetRequesterGadgets_tag {
  struct Gadget *tilesetListGadget;
} TilesetRequesterGadgets;

static void tilesetRequesterOnSelect(FrameworkWindow *tilesetRequester, UWORD selected) {
  MapEditorData *mapEditorData = tilesetRequester->parent->data;
  mapEditorDataSetTileset(mapEditorData, selected);
}

static ListViewSpec tilesetListSpec = {
  TILESET_LIST_LEFT,  TILESET_LIST_TOP,
  TILESET_REQUESTER_WIDTH  - TILESET_LIST_WIDTH_DELTA,
  TILESET_REQUESTER_HEIGHT - TILESET_LIST_HEIGHT_DELTA,
  NULL,
  NULL,
  tilesetRequesterOnSelect,
};

static WindowGadgets *createTilesetRequesterGadgets(WORD width, WORD height, void *data) {
  WindowGadgets *gadgets;
  TilesetRequesterGadgets *gadgetData;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "createTilesetRequesterGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  gadgetData = malloc(sizeof(TilesetRequesterGadgets));
  if(!gadgetData) {
    fprintf(stderr, "createTilesetRequesterGadgets: couldn't allocate data\n");
    goto error_freeWindowGadgets;
  }

  tilesetListSpec.height = height - TILESET_LIST_HEIGHT_DELTA;
  tilesetListSpec.width  = width  - TILESET_LIST_WIDTH_DELTA;
  tilesetListSpec.labels = ((TilesetRequesterData*)data)->tilesetNames;

  gadgets->glist = buildGadgets(
    makeListViewGadget(&tilesetListSpec), &gadgetData->tilesetListGadget,
    NULL);
  if(!gadgets->glist) {
    fprintf(stderr, "createTilesetRequesterGadgets: couldn't build gadgets\n");
    goto error_freeTilesetRequesterGadgets;
  }

  gadgets->data = gadgetData;

  return gadgets;

error_freeTilesetRequesterGadgets:
  free(gadgetData);
error_freeWindowGadgets:
  free(gadgets);
error:
  return NULL;
}

static void freeTilesetRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static void closeTilesetRequester(FrameworkWindow *tilesetRequester) {
  TilesetRequesterData *data = tilesetRequester->data;
  freeNumberedList(data->tilesetNames);
  free(data->title);
  free(data);
}

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
  (GadgetBuilder)    createTilesetRequesterGadgets,
  (GadgetFreer)      freeTilesetRequesterGadgets,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    closeTilesetRequester,
  (ClickFunction)    NULL,
};

BOOL isTilesetRequester(FrameworkWindow *window) {
  return (BOOL)(window->kind == &tilesetRequesterWindowKind);
}

FrameworkWindow *newTilesetRequester(FrameworkWindow *parent, const char *title) {
  FrameworkWindow *tilesetRequester;

  TilesetRequesterData *data = malloc(sizeof(TilesetRequesterData));
  if(!data) {
    fprintf(stderr, "newTilesetRequester: failed to allocate data\n");
    goto error;
  }

  data->title = strdup(title);
  if(!data->title) {
    fprintf(stderr, "newTilesetRequester: couldn't allocate title\n");
    goto error_freeData;
  }
  tilesetRequesterWindowKind.newWindow.Title = data->title;

  data->tilesetNames = newNumberedList(projectDataGetTilesetName, parent->parent->data, TILESETS);
  if(!data->tilesetNames) {
    fprintf(stderr, "newTilesetRequester: couldn't make tileset name list\n");
    goto error_freeTitle;
  }

  tilesetRequester = openChildWindow(parent, &tilesetRequesterWindowKind, data);
  if(!tilesetRequester) {
    fprintf(stderr, "newTilesetRequester: couldn't make window\n");
    goto error_freeNameList;
  }

  return tilesetRequester;
error_freeNameList:
  freeNumberedList(data->tilesetNames);
error_freeTitle:
  free(data->title);
error_freeData:
  free(data);
error:
  return NULL;
}

void tilesetRequesterRefresh(FrameworkWindow *tilesetRequester) {
  ProjectWindowData *projectData;
  TilesetRequesterData *data = tilesetRequester->data;
  TilesetRequesterGadgets *gadgets = tilesetRequester->gadgets->data;

  projectData = tilesetRequester->parent->parent->data;

  GT_SetGadgetAttrs(gadgets->tilesetListGadget, tilesetRequester->intuitionWindow, NULL,
    GTLV_Labels, ~0,
    TAG_END);

  freeNumberedList(data->tilesetNames);
  data->tilesetNames = newNumberedList(projectDataGetTilesetName, projectData, TILESETS);
  if(!data->tilesetNames) {
    fprintf(stderr, "tilesetRequesterRefresh: couldn't make tileset name list\n");
    goto error;
  }

  GT_SetGadgetAttrs(gadgets->tilesetListGadget, tilesetRequester->intuitionWindow, NULL,
    GTLV_Labels, data->tilesetNames,
    TAG_END);

  GT_RefreshWindow(tilesetRequester->intuitionWindow, NULL);

  return;
error:
  return;
}
