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
#define ENTITY_LIST_HEIGHT 200
#define ENTITY_LIST_LEFT   10
#define ENTITY_LIST_TOP    20

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

void initEntityBrowserScreen(void) {
    entityBrowserNewWindow.Screen = screen;
}

void initEntityBrowserVi(void) {
    entityListNewGadget.ng_VisualInfo = vi;
}

static void createEntityBrowserGadgets(EntityBrowser *entityBrowser) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
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
