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

#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "globals.h"
#include "ProjectWindow.h"
#include "TilesetPackage.h"

#define TILESET_REQUESTER_WIDTH      200
#define TILESET_REQUESTER_HEIGHT     336
#define TILESET_REQUESTER_MIN_HEIGHT 48

#define TILESET_LIST_WIDTH_DELTA  35
#define TILESET_LIST_HEIGHT_DELTA 26
#define TILESET_LIST_TOP          20
#define TILESET_LIST_LEFT         10

typedef struct TilesetRequesterData_tag {
  /* TODO: this should become a part of the FrameworkWindow */
  FrameworkWindow *parent;
} TilesetRequesterData;

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

static void createTilesetRequesterGadgets(TilesetRequester *tilesetRequester) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;
    int height = tilesetRequester->window ? tilesetRequester->window->intuitionWindow->Height : TILESET_REQUESTER_HEIGHT;
    int width  = tilesetRequester->window ? tilesetRequester->window->intuitionWindow->Width  : TILESET_REQUESTER_WIDTH;

    gad = CreateContext(&glist);

    tilesetListNewGadget.ng_Height = height - TILESET_LIST_HEIGHT_DELTA;
    tilesetListNewGadget.ng_Width  = width  - TILESET_LIST_WIDTH_DELTA;
    /* TODO: FIX ME */
/*    gad = CreateGadget(LISTVIEW_KIND, gad, &tilesetListNewGadget,
        GTLV_Labels, &tilesetPackage->tilesetNames,
        TAG_END); */
    tilesetRequester->tilesetList = gad;

    if(gad) {
        tilesetRequester->gadgets = glist;
	} else {
        tilesetRequester->tilesetList = NULL;
        FreeGadgets(glist);
        tilesetRequester->gadgets = NULL;
    }	
}

TilesetRequester *newTilesetRequester(char *title) {
    TilesetRequester *tilesetRequester = malloc(sizeof(TilesetRequester));
    if(!tilesetRequester) {
        goto error;
    }
    tilesetRequester->window = NULL;

    tilesetRequester->title = malloc(strlen(title) + 1);
    if(!tilesetRequester->title) {
        fprintf(stderr, "newTilesetRequester: couldn't allocate title\n");
        goto error_freeRequester;
    }
    strcpy(tilesetRequester->title, title);

    initTilesetRequesterVi();
    createTilesetRequesterGadgets(tilesetRequester);
    if(!tilesetRequester->gadgets) {
        goto error_freeTitle;
    }
    tilesetRequesterWindowKind.newWindow.FirstGadget = tilesetRequester->gadgets;

    tilesetRequesterWindowKind.newWindow.Title = tilesetRequester->title;
    tilesetRequester->window = openWindowOnGlobalScreen(&tilesetRequesterWindowKind);
    if(!tilesetRequester) {
        goto error_freeGadgets;
    }

    tilesetRequester->closed = 0;

    return tilesetRequester;
error_freeGadgets:
    FreeGadgets(tilesetRequester->gadgets);
error_freeTitle:
    free(tilesetRequester->title);
error_freeRequester:
    free(tilesetRequester);
error:
    return NULL;
}

void closeTilesetRequester(TilesetRequester *tilesetRequester) {
  /* TODO: the framework should free the gadgets */
    FreeGadgets(tilesetRequester->gadgets);
    free(tilesetRequester->title);
    free(tilesetRequester);
}

void refreshTilesetRequesterList(TilesetRequester *tilesetRequester) {
  TilesetRequesterData *data = tilesetRequester->window->data;
  ProjectWindowData *parentData = data->parent->data;
  TilesetPackage *tilesetPackage = parentData->tilesetPackage;

  GT_SetGadgetAttrs(tilesetRequester->tilesetList, tilesetRequester->window->intuitionWindow, NULL,
    GTLV_Labels, &tilesetPackage->tilesetNames,
    TAG_END);
}

void resizeTilesetRequester(TilesetRequester *tilesetRequester) {
    RemoveGList(tilesetRequester->window->intuitionWindow, tilesetRequester->gadgets, -1);
    FreeGadgets(tilesetRequester->gadgets);
    SetRast(tilesetRequester->window->intuitionWindow->RPort, 0);
    createTilesetRequesterGadgets(tilesetRequester);
    if(!tilesetRequester->gadgets) {
        fprintf(stderr, "resizeTilesetRequester: couldn't make gadgets");
        return;
    }
    AddGList(tilesetRequester->window->intuitionWindow, tilesetRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(tilesetRequester->window->intuitionWindow);
    RefreshGList(tilesetRequester->gadgets, tilesetRequester->window->intuitionWindow, NULL, -1);
    GT_RefreshWindow(tilesetRequester->window->intuitionWindow, NULL);
}
