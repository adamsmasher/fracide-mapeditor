#include "TilesetRequester.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

#define TILESET_REQUESTER_WIDTH  200
#define TILESET_REQUESTER_HEIGHT 336

#define TILESET_LIST_WIDTH        165
#define TILESET_LIST_HEIGHT_DELTA 26
#define TILESET_LIST_TOP          20
#define TILESET_LIST_LEFT         10

static struct NewWindow tilesetRequesterNewWindow = {
    40, 40, TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Choose Tileset", /* TODO: dynamically generate this based on map name */
    NULL,
    NULL,
    TILESET_REQUESTER_WIDTH, 16,
    TILESET_REQUESTER_WIDTH, 0xFFFF,
    CUSTOMSCREEN
};

static struct NewGadget tilesetListNewGadget = {
    TILESET_LIST_LEFT,  TILESET_LIST_TOP,
    TILESET_LIST_WIDTH, TILESET_REQUESTER_HEIGHT - TILESET_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    TILESET_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

void initTilesetRequesterScreen(void) {
    tilesetRequesterNewWindow.Screen = screen;
}

void initTilesetRequesterVi(void) {
    tilesetListNewGadget.ng_VisualInfo = vi;
}

static void createTilesetRequesterGadgets(TilesetRequester *tilesetRequester) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;
    int height = tilesetRequester->window ? tilesetRequester->window->Height : TILESET_REQUESTER_HEIGHT;

    gad = CreateContext(&glist);

    tilesetListNewGadget.ng_Height = height - TILESET_LIST_HEIGHT_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &tilesetListNewGadget,
        GTLV_Labels, &tilesetPackage->tilesetNames,
        TAG_END);
    tilesetRequester->tilesetList = gad;

    if(gad) {
        tilesetRequester->gadgets = glist;
	} else {
        tilesetRequester->tilesetList = NULL;
        FreeGadgets(glist);
        tilesetRequester->gadgets = NULL;
    }	
}

TilesetRequester *newTilesetRequester(void) {
    TilesetRequester *tilesetRequester = malloc(sizeof(TilesetRequester));
    if(!tilesetRequester) {
        goto error;
    }
    tilesetRequester->window = NULL;

    createTilesetRequesterGadgets(tilesetRequester);
    if(!tilesetRequester->gadgets) {
        goto error_freeRequester;
    }
    tilesetRequesterNewWindow.FirstGadget = tilesetRequester->gadgets;

    tilesetRequester->window = OpenWindow(&tilesetRequesterNewWindow);
    if(!tilesetRequester) {
        goto error_freeGadgets;
    }
    GT_RefreshWindow(tilesetRequester->window, NULL);

    tilesetRequester->closed = 0;

    return tilesetRequester;
error_freeGadgets:
    FreeGadgets(tilesetRequester->gadgets);
error_freeRequester:
    free(tilesetRequester);
error:
    return NULL;
}

void closeTilesetRequester(TilesetRequester *tilesetRequester) {
    CloseWindow(tilesetRequester->window);
    FreeGadgets(tilesetRequester->gadgets);
    free(tilesetRequester);
}

void refreshTilesetRequesterList(TilesetRequester *tilesetRequester) {
    GT_SetGadgetAttrs(tilesetRequester->tilesetList, tilesetRequester->window, NULL,
        GTLV_Labels, &tilesetPackage->tilesetNames,
        TAG_END);
}

void resizeTilesetRequester(TilesetRequester *tilesetRequester) {
    RemoveGList(tilesetRequester->window, tilesetRequester->gadgets, -1);
    FreeGadgets(tilesetRequester->gadgets);
    SetRast(tilesetRequester->window->RPort, 0);
    createTilesetRequesterGadgets(tilesetRequester);
    if(!tilesetRequester->gadgets) {
        fprintf(stderr, "resizeTilesetRequester: couldn't make gadgets");
        return;
    }
    AddGList(tilesetRequester->window, tilesetRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(tilesetRequester->window);
    RefreshGList(tilesetRequester->gadgets, tilesetRequester->window, NULL, -1);
    GT_RefreshWindow(tilesetRequester->window, NULL);
}
