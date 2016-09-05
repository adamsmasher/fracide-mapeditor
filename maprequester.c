#include "MapRequester.h"

#include <stdio.h>

#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include "globals.h"

#define MAP_REQUESTER_WIDTH  320
#define MAP_REQUESTER_HEIGHT 336

#define MAP_LIST_LEFT   20
#define MAP_LIST_TOP    20
#define MAP_LIST_WIDTH  268
#define MAP_LIST_HEIGHT 264

#define OK_BUTTON_LEFT   20
#define OK_BUTTON_TOP    300
#define OK_BUTTON_WIDTH  72
#define OK_BUTTON_HEIGHT 16

#define CANCEL_BUTTON_LEFT   216
#define CANCEL_BUTTON_TOP    300
#define CANCEL_BUTTON_WIDTH  72
#define CANCEL_BUTTON_HEIGHT 16

#define MAP_LIST_ID      1
#define OK_BUTTON_ID     (MAP_LIST_ID  + 1)
#define CANCEL_BUTTON_ID (OK_BUTTON_ID + 1)

/* TODO: make this resizeable up and down */
static struct NewWindow mapRequesterNewWindow = {
    40, 40, MAP_REQUESTER_WIDTH, MAP_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Save Map", /* TODO: dynamically generate this */
    NULL,
    NULL,
    MAP_REQUESTER_WIDTH, MAP_REQUESTER_HEIGHT,
    MAP_REQUESTER_WIDTH, MAP_REQUESTER_HEIGHT,
    CUSTOMSCREEN
};

/* TODO: get the font from the system preferences */
static struct TextAttr Topaz80 = { "topaz.font", 8, 0, 0 };

static struct NewGadget mapListNewGadget = {
	MAP_LIST_LEFT,  MAP_LIST_TOP,
	MAP_LIST_WIDTH, MAP_LIST_HEIGHT,
	NULL,
	&Topaz80,
	MAP_LIST_ID,
	0,
	NULL, /* visual info filled in later */
	NULL  /* user data */
};

static struct NewGadget okButtonNewGadget = {
    OK_BUTTON_LEFT,  OK_BUTTON_TOP,
    OK_BUTTON_WIDTH, OK_BUTTON_HEIGHT,
    "OK",
    &Topaz80,
    OK_BUTTON_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget cancelButtonNewGadget = {
    CANCEL_BUTTON_LEFT,  CANCEL_BUTTON_TOP,
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
    mapRequester->gadgets = NULL;

    gad = CreateContext(&mapRequester->gadgets);

    gad = CreateGadget(LISTVIEW_KIND, gad, &mapListNewGadget,
        /* TODO: get list of maps */
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &okButtonNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    mapRequester->okButton = gad;

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
            GA_Disabled, TRUE,
            TAG_END);
        break;
    }
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

int saveMapRequester(MapEditor *mapEditor) {
    MapRequester mapRequester;

    if(!Request(&requester, mapEditor->window)) {
        fprintf(stderr, "saveMapRequester: couldn't start requester\n");
        goto error;
    }

    createMapRequesterGadgets(&mapRequester);
    if(!mapRequester.gadgets) {
        fprintf(stderr, "saveMapRequester: couldn't create gadgets\n");
        goto error_EndRequest;
    }
    mapRequesterNewWindow.FirstGadget = mapRequester.gadgets;

    mapRequester.window = OpenWindow(&mapRequesterNewWindow);
    if(!mapRequester.window) {
        fprintf(stderr, "saveMapRequester: couldn't open window\n");
        goto error_FreeGadgets;
    }

    GT_RefreshWindow(mapRequester.window, NULL);

    requesterLoop(&mapRequester);

    CloseWindow(mapRequester.window);

    FreeGadgets(mapRequester.gadgets);

    EndRequest(&requester, mapEditor->window);

    return mapRequester.selected;

error_FreeGadgets:
    FreeGadgets(mapRequester.gadgets);
error_EndRequest:
    EndRequest(&requester, mapEditor->window);
error:
    return 0;
}