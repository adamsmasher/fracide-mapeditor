#include "MapRequester.h"

#include <stdio.h>

#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include "globals.h"

#define MAP_REQUESTER_WIDTH      320
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

#define MAP_LIST_ID      1
#define OK_BUTTON_ID     (MAP_LIST_ID  + 1)
#define CANCEL_BUTTON_ID (OK_BUTTON_ID + 1)

static struct NewWindow mapRequesterNewWindow = {
    40, 40, MAP_REQUESTER_WIDTH, MAP_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Save Map", /* TODO: dynamically generate this */
    NULL,
    NULL,
    MAP_REQUESTER_WIDTH, MAP_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
};

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

static struct NewGadget okButtonNewGadget = {
    OK_BUTTON_LEFT,  MAP_REQUESTER_HEIGHT - OK_BUTTON_BOTTOM_OFFSET,
    OK_BUTTON_WIDTH, OK_BUTTON_HEIGHT,
    "OK",
    &Topaz80,
    OK_BUTTON_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget cancelButtonNewGadget = {
    MAP_REQUESTER_WIDTH  - CANCEL_BUTTON_RIGHT_OFFSET,
    MAP_REQUESTER_HEIGHT - CANCEL_BUTTON_BOTTOM_OFFSET,
    CANCEL_BUTTON_WIDTH, CANCEL_BUTTON_HEIGHT,
    "Cancel",
    &Topaz80,
    CANCEL_BUTTON_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct Requester requester;

typedef struct MapRequester_tag {
    struct Window *window;
    int selected;
    int highlighted;
    struct Gadget *okButton;
    struct Gadget *gadgets;
} MapRequester;

void initMapRequesterScreen(void) {
    mapRequesterNewWindow.Screen = screen;
}

void initMapRequesterVi(void) {
    mapListNewGadget.ng_VisualInfo      = vi;
    okButtonNewGadget.ng_VisualInfo     = vi;
    cancelButtonNewGadget.ng_VisualInfo = vi;
}

static void createMapRequesterGadgets(MapRequester *mapRequester) {
    struct Gadget *gad;
    int height = mapRequester->window ? mapRequester->window->Height : MAP_REQUESTER_HEIGHT;
    int width  = mapRequester->window ? mapRequester->window->Width  : MAP_REQUESTER_WIDTH;
    mapRequester->gadgets = NULL;

    gad = CreateContext(&mapRequester->gadgets);

    mapListNewGadget.ng_Width  = width  - MAP_LIST_WIDTH_DELTA;
    mapListNewGadget.ng_Height = height - MAP_LIST_HEIGHT_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &mapListNewGadget,
        GTLV_Labels, &project.mapNames,
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

static void handleRequesterGadgetUp(MapRequester *mapRequester, struct Gadget *gadget, UWORD code) {
    switch(gadget->GadgetID) {
    case OK_BUTTON_ID:
        mapRequester->selected = mapRequester->highlighted + 1;
        break;
    case CANCEL_BUTTON_ID:
        mapRequester->selected = -1;
        break;
    case MAP_LIST_ID:
        mapRequester->highlighted = code;
        GT_SetGadgetAttrs(mapRequester->okButton, mapRequester->window, NULL,
            GA_Disabled, FALSE,
            TAG_END);
        break;
    }
}

static void resizeMapRequester(MapRequester *mapRequester) {
    RemoveGList(mapRequester->window, mapRequester->gadgets, -1);
    FreeGadgets(mapRequester->gadgets);
    SetRast(mapRequester->window->RPort, 0);
    createMapRequesterGadgets(mapRequester);
    if(!mapRequester->gadgets) {
        fprintf(stderr, "resizeMapRequester: couldn't make gadgets");
        return;
    }
    AddGList(mapRequester->window, mapRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(mapRequester->window);
    RefreshGList(mapRequester->gadgets, mapRequester->window, NULL, -1);
    GT_RefreshWindow(mapRequester->window, NULL);
}

static void handleRequesterMessage(MapRequester *mapRequester, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        mapRequester->selected = -1;
        break;
    case IDCMP_GADGETUP:
        handleRequesterGadgetUp(mapRequester, (struct Gadget*)msg->IAddress, msg->Code);
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(mapRequester->window);
        GT_EndRefresh(mapRequester->window, TRUE);
        break;
    case IDCMP_NEWSIZE:
        resizeMapRequester(mapRequester);
        break;
    }
}

static void handleRequesterMessages(MapRequester *mapRequester) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(mapRequester->window->UserPort)) {
        handleRequesterMessage(mapRequester, msg);
        GT_ReplyIMsg(msg);
    }
}

static void requesterLoop(MapRequester *mapRequester) {
    long signal = 1L << mapRequester->window->UserPort->mp_SigBit;
    mapRequester->selected = 0;
    while(!mapRequester->selected) {
        Wait(signal);
        handleRequesterMessages(mapRequester);
    }
    if(mapRequester->selected == -1) {
        mapRequester->selected = 0;
    }
}

static int spawnRequester(struct Window *window) {
    MapRequester mapRequester;
    mapRequester.window = NULL;

    if(!Request(&requester, window)) {
        fprintf(stderr, "spawnRequester: couldn't start requester\n");
        goto error;
    }

    createMapRequesterGadgets(&mapRequester);
    if(!mapRequester.gadgets) {
        fprintf(stderr, "spawnRequester: couldn't create gadgets\n");
        goto error_EndRequest;
    }
    mapRequesterNewWindow.FirstGadget = mapRequester.gadgets;

    mapRequester.window = OpenWindow(&mapRequesterNewWindow);
    if(!mapRequester.window) {
        fprintf(stderr, "spawnRequester: couldn't open window\n");
        goto error_FreeGadgets;
    }

    GT_RefreshWindow(mapRequester.window, NULL);

    requesterLoop(&mapRequester);

    CloseWindow(mapRequester.window);

    FreeGadgets(mapRequester.gadgets);

    EndRequest(&requester, window);

    return mapRequester.selected;

error_FreeGadgets:
    FreeGadgets(mapRequester.gadgets);
error_EndRequest:
    EndRequest(&requester, window);
error:
    return 0;
}

int openMapRequester(void) {
    mapRequesterNewWindow.Title = "Open Map";
    return spawnRequester(projectWindow);
}

int saveMapRequester(MapEditor *mapEditor) {
    /* TODO: dynamically generate me */
    mapRequesterNewWindow.Title = "Save Map";
    return spawnRequester(mapEditor->window);
}
