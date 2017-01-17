#include "EntityEditor.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"

#define ENTITY_EDITOR_WIDTH      200
#define ENTITY_EDITOR_HEIGHT     336
#define ENTITY_EDITOR_MIN_HEIGHT 48

#define ENTITY_LIST_WIDTH_DELTA  35
#define ENTITY_LIST_HEIGHT_DELTA 26
#define ENTITY_LIST_TOP          20
#define ENTITY_LIST_LEFT         10

#define ENTITY_NAME_WIDTH_DELTA   ENTITY_LIST_WIDTH_DELTA
#define ENTITY_NAME_HEIGHT        12
#define ENTITY_NAME_BOTTOM_OFFSET 26
#define ENTITY_NAME_LEFT          ENTITY_LIST_LEFT

static struct NewWindow entityEditorNewWindow = {
    40, 40, ENTITY_EDITOR_WIDTH, ENTITY_EDITOR_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Entity Editor",
    NULL,
    NULL,
    ENTITY_EDITOR_WIDTH, ENTITY_EDITOR_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
};

static struct NewGadget entityListNewGadget = {
    ENTITY_LIST_LEFT,  ENTITY_LIST_TOP,
    ENTITY_EDITOR_WIDTH - ENTITY_LIST_WIDTH_DELTA,
    ENTITY_EDITOR_HEIGHT - ENTITY_LIST_HEIGHT_DELTA,
    NULL,
    &Topaz80,
    ENTITY_EDITOR_LIST_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

static struct NewGadget entityNameNewGadget = {
    ENTITY_NAME_LEFT,  ENTITY_EDITOR_HEIGHT - ENTITY_NAME_BOTTOM_OFFSET,
    ENTITY_EDITOR_WIDTH - ENTITY_NAME_WIDTH_DELTA, ENTITY_NAME_HEIGHT,
    NULL,
    &Topaz80,
    ENTITY_NAME_ID,
    0,
    NULL, /* visual info filled in later */
    NULL  /* user data */
};

void initEntityEditorScreen(void) {
    entityEditorNewWindow.Screen = screen;
}

void initEntityEditorVi(void) {
    entityListNewGadget.ng_VisualInfo = vi;
    entityNameNewGadget.ng_VisualInfo = vi;
}

static void createEntityEditorGadgets(EntityEditor *entityEditor) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;
    int height = entityEditor->window ? entityEditor->window->Height : ENTITY_EDITOR_HEIGHT;
    int width  = entityEditor->window ? entityEditor->window->Width  : ENTITY_EDITOR_WIDTH;

    gad = CreateContext(&glist);

    if(entityEditor->editable) {
        entityNameNewGadget.ng_TopEdge = height - ENTITY_NAME_BOTTOM_OFFSET;
        entityNameNewGadget.ng_Width   = width  - ENTITY_NAME_WIDTH_DELTA;
        gad = CreateGadget(STRING_KIND, gad, &entityNameNewGadget,
            GTST_MaxChars, 64,
            GA_Disabled, TRUE);
        entityEditor->entityNameGadget = gad;
    } else {
        entityEditor->entityNameGadget = NULL;
    }

    entityListNewGadget.ng_Height = height - ENTITY_LIST_HEIGHT_DELTA;
    entityListNewGadget.ng_Width  = width  - ENTITY_LIST_WIDTH_DELTA;
    gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
        GTLV_ShowSelected, entityEditor->entityNameGadget,
        GTLV_Labels, &project.entityNames,
        TAG_END);

    if(gad) {
        entityEditor->gadgets = glist;
    } else {
        entityEditor->entityNameGadget = NULL;
        FreeGadgets(glist);
    }
}

static EntityEditor *newGenericEntityEditor(char *title, int editable) {
    EntityEditor *entityEditor = malloc(sizeof(EntityEditor));
    if(!entityEditor) {
        fprintf(stderr, "newGenericEntityEditor: couldn't allocate requester\n");
        goto error;
    }
    entityEditor->window = NULL;
    entityEditor->editable = editable;

    entityEditor->title = malloc(strlen(title) + 1);
    if(!entityEditor->title) {
        fprintf(stderr, "newGenericEntityEditor: couldn't allocate title\n");
        goto error_freeRequester;
    }
    strcpy(entityEditor->title, title);
    entityEditorNewWindow.Title = entityEditor->title;

    createEntityEditorGadgets(entityEditor);
    if(!entityEditor->gadgets) {
        fprintf(stderr, "newGenericEntityEditor: couldn't create gadgets\n");
        goto error_freeTitle;
    }
    entityEditorNewWindow.FirstGadget = entityEditor->gadgets;

    entityEditor->window = OpenWindow(&entityEditorNewWindow);
    if(!entityEditor->window) {
        fprintf(stderr, "newGenericEntityEditor: couldn't open window\n");
        goto error_freeGadgets;
    }
    GT_RefreshWindow(entityEditor->window, NULL);

    entityEditor->closed = 0;
    entityEditor->selected = 0;

    return entityEditor;

error_freeGadgets:
    free(entityEditor->gadgets);
error_freeTitle:
    free(entityEditor->title);
error_freeRequester:
    free(entityEditor);
error:
    return NULL;
}

#define EDITABLE 1
#define NON_EDITABLE 0

EntityEditor *newEntityEditor(void) {
    return newGenericEntityEditor("Entity Editor", EDITABLE);
}

void freeEntityEditor(EntityEditor *entityEditor) {
    CloseWindow(entityEditor->window);
    FreeGadgets(entityEditor->gadgets);
    free(entityEditor->title);
    free(entityEditor);
}

void resizeEntityEditor(EntityEditor *entityEditor) {
    RemoveGList(entityEditor->window, entityEditor->gadgets, -1);
    FreeGadgets(entityEditor->gadgets);
    SetRast(entityEditor->window->RPort, 0);
    createEntityEditorGadgets(entityEditor);
    if(!entityEditor->gadgets) {
        fprintf(stderr, "resizeEntityEditor: couldn't make gadgets");
        return;
    }
    AddGList(entityEditor->window, entityEditor->gadgets, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(entityEditor->window);
    RefreshGList(entityEditor->gadgets, entityEditor->window, NULL, -1);
    GT_RefreshWindow(entityEditor->window, NULL);
}
