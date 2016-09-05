#include "MapRequester.h"

#include <stdio.h>

#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include "globals.h"

#define MAP_REQUESTER_WIDTH  200
#define MAP_REQUESTER_HEIGHT 200

#define MAP_LIST_LEFT   20
#define MAP_LIST_TOP    20
#define MAP_LIST_WIDTH  160
#define MAP_LIST_HEIGHT 160

#define OK_BUTTON_LEFT   20
#define OK_BUTTON_TOP    20
#define OK_BUTTON_WIDTH  20
#define OK_BUTTON_HEIGHT 20

#define CANCEL_BUTTON_LEFT   20
#define CANCEL_BUTTON_TOP    20
#define CANCEL_BUTTON_WIDTH  20
#define CANCEL_BUTTON_HEIGHT 20

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

void initMapRequesterScreen(void) {
    mapRequesterNewWindow.Screen = screen;
}

void initMapRequesterVi(void) {
    mapListNewGadget.ng_VisualInfo      = vi;
    okButtonNewGadget.ng_VisualInfo     = vi;
    cancelButtonNewGadget.ng_VisualInfo = vi;
}

static struct Gadget *createMapRequesterGadgets(void) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    gad = CreateGadget(LISTVIEW_KIND, gad, &mapListNewGadget,
        /* TODO: get list of maps */
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &okButtonNewGadget,
        GA_Disabled, TRUE,
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &cancelButtonNewGadget, TAG_END);

    if(!gad) {
        goto error;
    }

    return glist;

error:
    FreeGadgets(glist);
    return NULL;
}

static void handleRequesterMessage(struct Window *window, struct IntuiMessage *msg, int *selected) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        *selected = -1;
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(window);
        GT_EndRefresh(window, TRUE);
        break;
    }
}

static int handleRequesterMessages(struct Window *window) {
    struct IntuiMessage *msg = NULL;
    int selected = 0;
    while(msg = GT_GetIMsg(window->UserPort)) {
        handleRequesterMessage(window, msg, &selected);
        GT_ReplyIMsg(msg);
    }
    return selected;
}

static int requesterLoop(struct Window *window) {
    long signal = 1L << window->UserPort->mp_SigBit;
    int selected = 0;
    while(!selected) {
        Wait(signal);
        selected = handleRequesterMessages(window);
    }
    if(selected == -1) {
        return 0;
    } else {
        return selected;
    }
}

int saveMapRequester(MapEditor *mapEditor) {
    int result;
    struct Window *window;
    struct Gadget *glist;

    if(!Request(&requester, mapEditor->window)) {
        fprintf(stderr, "saveMapRequester: couldn't start requester\n");
        goto error;
    }

    glist = createMapRequesterGadgets();
    if(!glist) {
        fprintf(stderr, "saveMapRequester: couldn't create gadgets\n");
        goto error_EndRequest;
    }
    mapRequesterNewWindow.FirstGadget = glist;

    window = OpenWindow(&mapRequesterNewWindow);
    if(!window) {
        fprintf(stderr, "saveMapRequester: couldn't open window\n");
        goto error_FreeGadgets;
    }

    GT_RefreshWindow(window, NULL);

    result = requesterLoop(window);

    CloseWindow(window);

    FreeGadgets(glist);

    EndRequest(&requester, mapEditor->window);

    return result;

error_FreeGadgets:
    FreeGadgets(glist);
error_EndRequest:
    EndRequest(&requester, mapEditor->window);
error:
    return 0;
}