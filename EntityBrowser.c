#include "EntityBrowser.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#define ENTITY_BROWSER_WIDTH  350
#define ENTITY_BROWSER_HEIGHT 225

#define ENTITY_LIST_WIDTH  120
#define ENTITY_LIST_HEIGHT 170
#define ENTITY_LIST_LEFT   10
#define ENTITY_LIST_TOP    20

#define ADD_ENTITY_WIDTH   120
#define ADD_ENTITY_HEIGHT  14
#define ADD_ENTITY_LEFT    10
#define ADD_ENTITY_TOP     190

#define REMOVE_ENTITY_WIDTH  120
#define REMOVE_ENTITY_HEIGHT 14
#define REMOVE_ENTITY_LEFT   10
#define REMOVE_ENTITY_TOP    205

#define THIS_ENTITY_WIDTH    102
#define THIS_ENTITY_HEIGHT   14
#define THIS_ENTITY_LEFT     200
#define THIS_ENTITY_TOP      25

#define CHOOSE_ENTITY_WIDTH  32
#define CHOOSE_ENTITY_HEIGHT 14
#define CHOOSE_ENTITY_LEFT   300
#define CHOOSE_ENTITY_TOP    25

#define ENTITY_ROW_WIDTH     32
#define ENTITY_ROW_HEIGHT    14
#define ENTITY_ROW_LEFT      175
#define ENTITY_ROW_TOP       50

#define ENTITY_COL_WIDTH     32
#define ENTITY_COL_HEIGHT    14
#define ENTITY_COL_LEFT      300
#define ENTITY_COL_TOP       50

#define VRAM_SLOT_WIDTH      48
#define VRAM_SLOT_HEIGHT     14
#define VRAM_SLOT_LEFT       223
#define VRAM_SLOT_TOP        75

static struct NewWindow entityBrowserNewWindow = {
    40, 40, ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|ACTIVATE,
    NULL,
    NULL,
    "Entities",
    NULL,
    NULL,
    ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    CUSTOMSCREEN
};

static struct NewGadget entityListNewGadget = {
    ENTITY_LIST_LEFT, ENTITY_LIST_TOP,
    ENTITY_LIST_WIDTH, ENTITY_LIST_HEIGHT,
    NULL,
    &Topaz80,
    ENTITY_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget addEntityNewGadget = {
    ADD_ENTITY_LEFT, ADD_ENTITY_TOP,
    ADD_ENTITY_WIDTH, ADD_ENTITY_HEIGHT,
    "Add Entity...",
    &Topaz80,
    ADD_ENTITY_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget removeEntityNewGadget = {
    REMOVE_ENTITY_LEFT, REMOVE_ENTITY_TOP,
    REMOVE_ENTITY_WIDTH, REMOVE_ENTITY_HEIGHT,
    "Remove Entity",
    &Topaz80,
    REMOVE_ENTITY_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget entityRowNewGadget = {
    ENTITY_ROW_LEFT, ENTITY_ROW_TOP,
    ENTITY_ROW_WIDTH, ENTITY_ROW_HEIGHT,
    "Row",
    &Topaz80,
    ENTITY_ROW_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget entityColNewGadget = {
    ENTITY_COL_LEFT, ENTITY_COL_TOP,
    ENTITY_COL_WIDTH, ENTITY_COL_HEIGHT,
    "Column",
    &Topaz80,
    ENTITY_COL_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget VRAMSlotNewGadget = {
    VRAM_SLOT_LEFT, VRAM_SLOT_TOP,
    VRAM_SLOT_WIDTH, VRAM_SLOT_HEIGHT,
    "VRAM Slot",
    &Topaz80,
    VRAM_SLOT_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget thisEntityNewGadget = {
    THIS_ENTITY_LEFT, THIS_ENTITY_TOP,
    THIS_ENTITY_WIDTH, THIS_ENTITY_HEIGHT,
    "Entity",
    &Topaz80,
    THIS_ENTITY_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget chooseEntityNewGadget = {
    CHOOSE_ENTITY_LEFT, CHOOSE_ENTITY_TOP,
    CHOOSE_ENTITY_WIDTH, CHOOSE_ENTITY_HEIGHT,
    "...",
    &Topaz80,
    CHOOSE_ENTITY_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget *allNewGadgets[] = {
    &entityListNewGadget,
    &addEntityNewGadget,
    &removeEntityNewGadget,
    &thisEntityNewGadget,
    &chooseEntityNewGadget,
    &entityRowNewGadget,
    &entityColNewGadget,
    &VRAMSlotNewGadget,
    NULL
};

void initEntityBrowserScreen(void) {
    entityBrowserNewWindow.Screen = screen;
}

void initEntityBrowserVi(void) {
    struct NewGadget **i = allNewGadgets;
    while(*i) {
        (*i)->ng_VisualInfo = vi;
        i++;
    }
}

static void createEntityBrowserGadgets(EntityBrowser *entityBrowser) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &addEntityNewGadget, TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &removeEntityNewGadget,
        GA_Disabled, TRUE,
        TAG_END);

    gad = CreateGadget(TEXT_KIND, gad, &thisEntityNewGadget,
        GTTX_Text, "N/A",
        GTTX_Border, TRUE,
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &chooseEntityNewGadget, TAG_END);

    gad = CreateGadget(INTEGER_KIND, gad, &entityRowNewGadget,
        GTIN_MaxChars, 1,
        TAG_END);

    gad = CreateGadget(INTEGER_KIND, gad, &entityColNewGadget,
        GTIN_MaxChars, 1,
        TAG_END);

    gad = CreateGadget(INTEGER_KIND, gad, &VRAMSlotNewGadget,
        GTIN_MaxChars, 3,
        TAG_END);

    if(gad) {
        entityBrowser->gadgets = glist;
    } else {
        FreeGadgets(glist);
    }
}

EntityBrowser *newEntityBrowser(char *title) {
    EntityBrowser *entityBrowser = malloc(sizeof(EntityBrowser));
    if(!entityBrowser) {
        fprintf(stderr, "newEntityBrowser: couldn't allocate entity browser\n");
        goto error;
    }

    entityBrowser->title = malloc(strlen(title) + 1);
    if(!entityBrowser->title) {
        fprintf(stderr, "newEntityBrowser: couldn't allocate title\n");
        goto error_freeBrowser;
    }
    strcpy(entityBrowser->title, title);
    entityBrowserNewWindow.Title = entityBrowser->title;

    createEntityBrowserGadgets(entityBrowser);
    if(!entityBrowser->gadgets) {
        fprintf(stderr, "newEntityBrowser: couldn't create gadgets\n");
        goto error_freeTitle;
    }
    entityBrowserNewWindow.FirstGadget = entityBrowser->gadgets;
    
    entityBrowser->window = OpenWindow(&entityBrowserNewWindow);
    if(!entityBrowser->window) {
        fprintf(stderr, "newEntityBrowser: couldn't open window\n");
        goto error_freeGadgets;
    }
    GT_RefreshWindow(entityBrowser->window, NULL);

    entityBrowser->closed = 0;

    return entityBrowser;
    
error_freeGadgets:
    free(entityBrowser->gadgets);
error_freeTitle:
    free(entityBrowser->title);
error_freeBrowser:
    free(entityBrowser);
error:
    return NULL;
}

void freeEntityBrowser(EntityBrowser *entityBrowser) {
    CloseWindow(entityBrowser->window);
    FreeGadgets(entityBrowser->gadgets);
    free(entityBrowser->title);
    free(entityBrowser);
}
