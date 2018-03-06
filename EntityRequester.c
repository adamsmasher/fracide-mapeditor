#include "EntityRequester.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
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
#include "ProjectWindowData.h"

#define ENTITY_REQUESTER_WIDTH      200
#define ENTITY_REQUESTER_HEIGHT     336
#define ENTITY_REQUESTER_MIN_HEIGHT 48

#define ENTITY_LIST_WIDTH_DELTA  35
#define ENTITY_LIST_HEIGHT_DELTA 26
#define ENTITY_LIST_TOP          20
#define ENTITY_LIST_LEFT         10

#define ENTITY_NAME_WIDTH_DELTA   ENTITY_LIST_WIDTH_DELTA
#define ENTITY_NAME_HEIGHT        12
#define ENTITY_NAME_BOTTOM_OFFSET 26
#define ENTITY_NAME_LEFT          ENTITY_LIST_LEFT

static WindowKind entityRequesterWindowKind = {
  {
    40, 40, ENTITY_REQUESTER_WIDTH, ENTITY_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Entity Editor",
    NULL,
    NULL,
    ENTITY_REQUESTER_WIDTH, ENTITY_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL
};

static struct NewGadget entityListNewGadget = {
    ENTITY_LIST_LEFT,  ENTITY_LIST_TOP,
    ENTITY_REQUESTER_WIDTH - ENTITY_LIST_WIDTH_DELTA,
    ENTITY_REQUESTER_HEIGHT - ENTITY_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    ENTITY_REQUESTER_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget entityNameNewGadget = {
    ENTITY_NAME_LEFT,  ENTITY_REQUESTER_HEIGHT - ENTITY_NAME_BOTTOM_OFFSET,
    ENTITY_REQUESTER_WIDTH - ENTITY_NAME_WIDTH_DELTA, ENTITY_NAME_HEIGHT,
    NULL,
    &Topaz80,
    ENTITY_NAME_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static void initEntityRequesterVi(void) {
  void *vi = getGlobalVi();
  if(!vi) {
    fprintf(stderr, "initEntityRequesterVi: failed to get visual info\n");
  }

  entityListNewGadget.ng_VisualInfo = vi;
  entityNameNewGadget.ng_VisualInfo = vi;
}

static void createEntityRequesterGadgets(EntityRequester *entityRequester) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;
    ProjectWindowData *projectData = NULL; /* TODO: fix me */
    int height = entityRequester->window ? entityRequester->window->intuitionWindow->Height : ENTITY_REQUESTER_HEIGHT;
    int width  = entityRequester->window ? entityRequester->window->intuitionWindow->Width  : ENTITY_REQUESTER_WIDTH;

    gad = CreateContext(&glist);

    if(entityRequester->editable) {
        entityNameNewGadget.ng_TopEdge = height - ENTITY_NAME_BOTTOM_OFFSET;
        entityNameNewGadget.ng_Width   = width  - ENTITY_NAME_WIDTH_DELTA;
        gad = CreateGadget(STRING_KIND, gad, &entityNameNewGadget,
            GTST_MaxChars, 64,
            GA_Disabled, TRUE);
        entityRequester->entityNameGadget = gad;
    } else {
        entityRequester->entityNameGadget = NULL;
    }

    entityListNewGadget.ng_Height = height - ENTITY_LIST_HEIGHT_DELTA;
    entityListNewGadget.ng_Width  = width  - ENTITY_LIST_WIDTH_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
        GTLV_ShowSelected, entityRequester->entityNameGadget,
        GTLV_Labels, projectDataGetEntityNames(projectData),
        TAG_END);

    if(gad) {
        entityRequester->gadgets = glist;
    } else {
        entityRequester->entityNameGadget = NULL;
        FreeGadgets(glist);
    }
}

static EntityRequester *newGenericEntityRequester(char *title, int editable) {
    EntityRequester *entityRequester = malloc(sizeof(EntityRequester));
    if(!entityRequester) {
        fprintf(stderr, "newGenericEntityRequester: couldn't allocate requester\n");
        goto error;
    }
    entityRequester->window = NULL;
    entityRequester->editable = editable;

    entityRequesterWindowKind.newWindow.Title = title;

    initEntityRequesterVi();
    createEntityRequesterGadgets(entityRequester);
    if(!entityRequester->gadgets) {
        fprintf(stderr, "newGenericEntityRequester: couldn't create gadgets\n");
        goto error_freeRequester;
    }
    entityRequesterWindowKind.newWindow.FirstGadget = entityRequester->gadgets;

    entityRequester->window = openWindowOnGlobalScreen(&entityRequesterWindowKind);
    if(!entityRequester->window) {
        fprintf(stderr, "newGenericEntityRequester: couldn't open window\n");
        goto error_freeGadgets;
    }

    entityRequester->closed = 0;
    entityRequester->selected = 0;

    return entityRequester;

error_freeGadgets:
    free(entityRequester->gadgets);
error_freeRequester:
    free(entityRequester);
error:
    return NULL;
}

#define EDITABLE 1
#define NON_EDITABLE 0

EntityRequester *newEntityNamesEditor(void) {
    return newGenericEntityRequester("Entity Names Editor", EDITABLE);
}

EntityRequester *newEntityRequester(void) {
    return newGenericEntityRequester("Choose Entity...", NON_EDITABLE);
}

void freeEntityRequester(EntityRequester *entityRequester) {
    /* TODO: fix me */
    CloseWindow(entityRequester->window->intuitionWindow);
    FreeGadgets(entityRequester->gadgets);
    free(entityRequester);
}

void resizeEntityRequester(EntityRequester *entityRequester) {
    RemoveGList(entityRequester->window->intuitionWindow, entityRequester->gadgets, -1);
    FreeGadgets(entityRequester->gadgets);
    SetRast(entityRequester->window->intuitionWindow->RPort, 0);
    createEntityRequesterGadgets(entityRequester);
    if(!entityRequester->gadgets) {
        fprintf(stderr, "resizeEntityRequester: couldn't make gadgets");
        return;
    }
    AddGList(entityRequester->window->intuitionWindow, entityRequester->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(entityRequester->window->intuitionWindow);
    RefreshGList(entityRequester->gadgets, entityRequester->window->intuitionWindow, NULL, -1);
    GT_RefreshWindow(entityRequester->window->intuitionWindow, NULL);
}
