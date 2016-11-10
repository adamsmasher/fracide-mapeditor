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

#define ENTITY_BROWSER_WIDTH  200
#define ENTITY_BROWSER_HEIGHT 336

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

void initEntityBrowserScreen(void) {
    entityBrowserNewWindow.Screen = screen;
}

void initEntityBrowserVi(void) {
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
    entityBrowserNewWindow.Title = title;
    
    entityBrowser->window = OpenWindow(&entityBrowserNewWindow);
    if(!entityBrowser->window) {
        fprintf(stderr, "newEntityBrowser: couldn't open window\n");
        goto error_freeTitle;
    }

    entityBrowser->closed = 0;

    return entityBrowser;
    
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
