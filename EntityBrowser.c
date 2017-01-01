#include "EntityBrowser.h"

#include <proto/exec.h>

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

#define TAG_LIST_WIDTH       190
#define TAG_LIST_HEIGHT      50
#define TAG_LIST_LEFT        143
#define TAG_LIST_TOP         100

#define ADD_TAG_WIDTH        190
#define ADD_TAG_HEIGHT       14
#define ADD_TAG_LEFT         143
#define ADD_TAG_TOP          150

#define DELETE_TAG_WIDTH     190
#define DELETE_TAG_HEIGHT    14
#define DELETE_TAG_LEFT      143
#define DELETE_TAG_TOP       165

#define TAG_ALIAS_WIDTH      143
#define TAG_ALIAS_HEIGHT     14
#define TAG_ALIAS_LEFT       190
#define TAG_ALIAS_TOP        185

#define TAG_ID_WIDTH         48
#define TAG_ID_HEIGHT        14
#define TAG_ID_LEFT          165
#define TAG_ID_TOP           205

#define TAG_VALUE_WIDTH      48
#define TAG_VALUE_HEIGHT     14
#define TAG_VALUE_LEFT       285
#define TAG_VALUE_TOP        205

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

static struct NewGadget tagListNewGadget = {
    TAG_LIST_LEFT, TAG_LIST_TOP,
    TAG_LIST_WIDTH, TAG_LIST_HEIGHT,
    NULL,
    &Topaz80,
    TAG_LIST_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget addTagNewGadget = {
    ADD_TAG_LEFT, ADD_TAG_TOP,
    ADD_TAG_WIDTH, ADD_TAG_HEIGHT,
    "Add Tag",
    &Topaz80,
    ADD_TAG_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget deleteTagNewGadget = {
    DELETE_TAG_LEFT, DELETE_TAG_TOP,
    DELETE_TAG_WIDTH, DELETE_TAG_HEIGHT,
    "Delete Tag",
    &Topaz80,
    DELETE_TAG_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget tagAliasNewGadget = {
    TAG_ALIAS_LEFT, TAG_ALIAS_TOP,
    TAG_ALIAS_WIDTH, TAG_ALIAS_HEIGHT,
    "Alias",
    &Topaz80,
    TAG_ALIAS_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget tagIdNewGadget = {
    TAG_ID_LEFT, TAG_ID_TOP,
    TAG_ID_WIDTH, TAG_ID_HEIGHT,
    "ID",
    &Topaz80,
    TAG_ID_ID,
    0,
    NULL,
    NULL
};

static struct NewGadget tagValueNewGadget = {
    TAG_VALUE_LEFT, TAG_VALUE_TOP,
    TAG_VALUE_WIDTH, TAG_VALUE_HEIGHT,
    "Value",
    &Topaz80,
    TAG_VALUE_ID,
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
    &tagListNewGadget,
    &addTagNewGadget,
    &deleteTagNewGadget,
    &tagAliasNewGadget,
    &tagIdNewGadget,
    &tagValueNewGadget,
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

static void createEntityBrowserGadgets(EntityBrowser *entityBrowser, int entityCnt) {
    struct Gadget *gad;
    struct Gadget *glist = NULL;

    gad = CreateContext(&glist);

    gad = CreateGadget(LISTVIEW_KIND, gad, &entityListNewGadget,
        GTLV_Labels, &entityBrowser->entityLabels,
        TAG_END);
    entityBrowser->entityListGadget = gad;

    if(entityCnt < MAX_ENTITIES_PER_MAP) {
        gad = CreateGadget(BUTTON_KIND, gad, &addEntityNewGadget, TAG_END);
    } else {
        gad = CreateGadget(BUTTON_KIND, gad, &addEntityNewGadget,
            GA_Disabled, TRUE,
            TAG_END);
    }
    entityBrowser->addEntityGadget = gad;

    gad = CreateGadget(BUTTON_KIND, gad, &removeEntityNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->removeEntityGadget = gad;

    gad = CreateGadget(TEXT_KIND, gad, &thisEntityNewGadget,
        GTTX_Text, "N/A",
        GTTX_Border, TRUE,
        TAG_END);

    gad = CreateGadget(BUTTON_KIND, gad, &chooseEntityNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->chooseEntityGadget = gad;

    gad = CreateGadget(INTEGER_KIND, gad, &entityRowNewGadget,
        GTIN_MaxChars, 1,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->rowGadget = gad;

    gad = CreateGadget(INTEGER_KIND, gad, &entityColNewGadget,
        GTIN_MaxChars, 1,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->colGadget = gad;

    gad = CreateGadget(INTEGER_KIND, gad, &VRAMSlotNewGadget,
        GTIN_MaxChars, 3,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->VRAMSlotGadget = gad;

    gad = CreateGadget(LISTVIEW_KIND, gad, &tagListNewGadget, TAG_END);
    entityBrowser->tagListGadget = gad;

    gad = CreateGadget(BUTTON_KIND, gad, &addTagNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->addTagGadget = gad;

    gad = CreateGadget(BUTTON_KIND, gad, &deleteTagNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->deleteTagGadget = gad;

    gad = CreateGadget(STRING_KIND, gad, &tagAliasNewGadget,
        GA_Disabled, TRUE,
        TAG_END);
    entityBrowser->tagAliasGadget = gad;

    gad = CreateGadget(INTEGER_KIND, gad, &tagIdNewGadget,
        GA_Disabled, TRUE,
        GTIN_MaxChars, 3,
        TAG_END);
    entityBrowser->tagIdGadget = gad;

    gad = CreateGadget(INTEGER_KIND, gad, &tagValueNewGadget,
        GA_Disabled, TRUE,
        GTIN_MaxChars, 3,
        TAG_END);
    entityBrowser->tagValueGadget = gad;

    if(gad) {
        entityBrowser->gadgets = glist;
    } else {
        entityBrowser->addEntityGadget = NULL;
        entityBrowser->removeEntityGadget = NULL;
        entityBrowser->entityListGadget = NULL;
        entityBrowser->rowGadget = NULL;
        entityBrowser->colGadget = NULL;
        entityBrowser->VRAMSlotGadget = NULL;
        entityBrowser->addTagGadget = NULL;
        entityBrowser->chooseEntityGadget = NULL;
        entityBrowser->tagListGadget = NULL;
        entityBrowser->tagIdGadget = NULL;
        entityBrowser->tagValueGadget = NULL;
        entityBrowser->tagAliasGadget = NULL;
        entityBrowser->deleteTagGadget = NULL;
        FreeGadgets(glist);
    }
}

static int createTagLabels(EntityBrowser *entityBrowser, Frac_tag *tags, int tagCnt) {
    int i;

    NewList(&entityBrowser->tagLabels);

    if(!tagCnt) {
        entityBrowser->tagNodes   = NULL;
        entityBrowser->tagStrings = NULL;
        goto ok;
    }

    entityBrowser->tagNodes = malloc(sizeof(struct Node) * tagCnt);
    if(!entityBrowser->tagNodes) {
        fprintf(stderr, "createTagLabels: couldn't allocate memory for %d nodes\n", tagCnt);
        goto error;
    }

    entityBrowser->tagStrings = malloc((TAG_ALIAS_LENGTH + 4) * tagCnt);
    if(!entityBrowser->tagStrings) {
        fprintf(stderr, "createTagLabels: couldn't allocate memory for labels\n");
        goto error_freeNodes;
    }

    for(i = 0; i < tagCnt; i++) {
        sprintf(entityBrowser->tagStrings[i], "%d: %s", i, tags[i].alias);
        entityBrowser->tagNodes[i].ln_Name = entityBrowser->tagStrings[i];
        AddTail(&entityBrowser->tagLabels, &entityBrowser->tagNodes[i]);
    }

ok:
    return 1;

error_freeNodes:
    free(entityBrowser->tagNodes);
    entityBrowser->tagNodes = NULL;
error:
    entityBrowser->tagStrings = NULL;
    return 0;
}

static int createEntityLabels(EntityBrowser *entityBrowser, Entity *entities, int entityCnt) {
    int i;

    NewList(&entityBrowser->entityLabels);

    if(!entityCnt) {
        entityBrowser->entityNodes   = NULL;
        entityBrowser->entityStrings = NULL;
        goto ok;
    }

    entityBrowser->entityNodes = malloc(sizeof(struct Node) * entityCnt);
    if(!entityBrowser->entityNodes) {
        fprintf(stderr, "createEntityLabels: couldn't allocate memory for %d nodes\n", entityCnt);
        goto error;
    }

    entityBrowser->entityStrings = malloc(ENTITY_LABEL_LENGTH * entityCnt);
    if(!entityBrowser->entityStrings) {
        fprintf(stderr, "createEntityLabels: couldn't allocate memory for labels\n");
        goto error_freeNodes;
    }

    for(i = 0; i < entityCnt; i++) {
        sprintf(entityBrowser->entityStrings[i], "%d: N/A", i);
        entityBrowser->entityNodes[i].ln_Name = entityBrowser->entityStrings[i];
        AddTail(&entityBrowser->entityLabels, &entityBrowser->entityNodes[i]);
    }

ok:
    return 1;

error_freeNodes:
    free(entityBrowser->entityNodes);
    entityBrowser->entityNodes = NULL;
error:
    entityBrowser->entityStrings = NULL;
    return 0;
}

EntityBrowser *newEntityBrowser(char *title, Entity *entities, int entityCnt) {
    EntityBrowser *entityBrowser = malloc(sizeof(EntityBrowser));
    if(!entityBrowser) {
        fprintf(stderr, "newEntityBrowser: couldn't allocate entity browser\n");
        goto error;
    }

    if(!createEntityLabels(entityBrowser, entities, entityCnt)) {
        fprintf(stderr, "newEntityBrowser: couldn't create labels\n");
        goto error_freeBrowser;
    }

    entityBrowser->title = malloc(strlen(title) + 1);
    if(!entityBrowser->title) {
        fprintf(stderr, "newEntityBrowser: couldn't allocate title\n");
        goto error_freeLabels;
    }
    strcpy(entityBrowser->title, title);
    entityBrowserNewWindow.Title = entityBrowser->title;

    createEntityBrowserGadgets(entityBrowser, entityCnt);
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
    entityBrowser->selectedEntity = 0;
    entityBrowser->selectedTag = 0;

    return entityBrowser;
    
error_freeGadgets:
    free(entityBrowser->gadgets);
error_freeTitle:
    free(entityBrowser->title);
error_freeLabels:
    free(entityBrowser->entityNodes);
    free(entityBrowser->entityStrings);
error_freeBrowser:
    free(entityBrowser);
error:
    return NULL;
}

static void freeEntityLabels(EntityBrowser *entityBrowser) {
    free(entityBrowser->entityNodes);
    free(entityBrowser->entityStrings);
}

static void freeTagLabels(EntityBrowser *entityBrowser) {
    free(entityBrowser->tagNodes);
    free(entityBrowser->tagStrings);
}

void freeEntityBrowser(EntityBrowser *entityBrowser) {
    CloseWindow(entityBrowser->window);
    FreeGadgets(entityBrowser->gadgets);
    free(entityBrowser->title);
    freeEntityLabels(entityBrowser);
    freeTagLabels(entityBrowser);
    free(entityBrowser);
}

int entityBrowserSetTags(EntityBrowser *entityBrowser, Frac_tag *tags, int tagCnt) {
    if(!createTagLabels(entityBrowser, tags, tagCnt)) {
        fprintf(stderr, "entityBrowserSetTags: couldn't create tag labels\n");
        goto error;
    }

    GT_SetGadgetAttrs(entityBrowser->tagListGadget, entityBrowser->window, NULL,
        GTLV_Labels, &entityBrowser->tagLabels,
        TAG_END);

    return 1;
error:
    return 0;
}

void entityBrowserFreeTagLabels(EntityBrowser *entityBrowser) {
    GT_SetGadgetAttrs(entityBrowser->tagListGadget, entityBrowser->window, NULL,
        GTLV_Labels, NULL,
        TAG_END);

    freeTagLabels(entityBrowser);
}

void entityBrowserSelectTag(EntityBrowser *entityBrowser, int tagNum, Frac_tag *tag) {
    entityBrowser->selectedTag = tagNum + 1;

    GT_SetGadgetAttrs(entityBrowser->deleteTagGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->tagAliasGadget, entityBrowser->window, NULL,
        GTST_String, tag->alias,
        GA_Disabled, FALSE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->tagIdGadget, entityBrowser->window, NULL,
        GTIN_Number, tag->id,
        GA_Disabled, FALSE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->tagValueGadget, entityBrowser->window, NULL,
        GTIN_Number, tag->value,
        GA_Disabled, FALSE,
        TAG_END);
}

void entityBrowserDeselectTag(EntityBrowser *entityBrowser) {
    GT_SetGadgetAttrs(entityBrowser->tagAliasGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->tagIdGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->tagValueGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->deleteTagGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);
}

void entityBrowserSelectEntity(EntityBrowser *entityBrowser, int entityNum, Entity *entity) {
    entityBrowser->selectedEntity = entityNum + 1;
    entityBrowser->selectedTag = 0;

    GT_SetGadgetAttrs(entityBrowser->removeEntityGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->rowGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        GTIN_Number, entity->row,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->colGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        GTIN_Number, entity->col,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->VRAMSlotGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        GTIN_Number, entity->vramSlot,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->addTagGadget, entityBrowser->window, NULL,
        GA_Disabled, entity->tagCnt >= MAX_TAGS_PER_ENTITY,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->chooseEntityGadget, entityBrowser->window, NULL,
        GA_Disabled, FALSE,
        TAG_END);

    entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);
    entityBrowserDeselectTag(entityBrowser);
}

void entityBrowserDeselectEntity(EntityBrowser *entityBrowser) {
    entityBrowser->selectedEntity = 0;
    entityBrowser->selectedTag = 0;

    GT_SetGadgetAttrs(entityBrowser->removeEntityGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->rowGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->colGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->VRAMSlotGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->addTagGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    GT_SetGadgetAttrs(entityBrowser->chooseEntityGadget, entityBrowser->window, NULL,
        GA_Disabled, TRUE,
        TAG_END);

    entityBrowserDeselectTag(entityBrowser);
    entityBrowserFreeTagLabels(entityBrowser);
}

int entityBrowserSetEntities(EntityBrowser *entityBrowser, Entity *entities, int entityCnt) {
    if(!createEntityLabels(entityBrowser, entities, entityCnt)) {
        fprintf(stderr, "entityBrowserSetEntities: couldn't create entity labels\n");
        goto error;
    }

    GT_SetGadgetAttrs(entityBrowser->entityListGadget, entityBrowser->window, NULL,
        GTLV_Labels, &entityBrowser->entityLabels,
        TAG_END);
    
    return 1;
error:
    return 0;
}

void entityBrowserFreeEntityLabels(EntityBrowser *entityBrowser) {
    GT_SetGadgetAttrs(entityBrowser->entityListGadget, entityBrowser->window, NULL,
        GTLV_Labels, ~0,
        TAG_END);

    freeEntityLabels(entityBrowser);
}
