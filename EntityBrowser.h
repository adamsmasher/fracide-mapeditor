#ifndef ENTITY_BROWSER_H
#define ENTITY_BROWSER_H

#include <proto/intuition.h>

#include "Entity.h"

#define ENTITY_LIST_ID   0
#define ADD_ENTITY_ID    (ENTITY_LIST_ID   + 1)
#define REMOVE_ENTITY_ID (ADD_ENTITY_ID    + 1)
#define THIS_ENTITY_ID   (REMOVE_ENTITY_ID + 1)
#define CHOOSE_ENTITY_ID (THIS_ENTITY_ID   + 1)
#define ENTITY_ROW_ID    (CHOOSE_ENTITY_ID + 1)
#define ENTITY_COL_ID    (ENTITY_ROW_ID    + 1)
#define VRAM_SLOT_ID     (ENTITY_COL_ID    + 1)
#define TAG_LIST_ID      (VRAM_SLOT_ID     + 1)
#define TAG_ALIAS_ID     (TAG_LIST_ID      + 1)
#define TAG_ID_ID        (TAG_ALIAS_ID     + 1)
#define TAG_VALUE_ID     (TAG_ID_ID        + 1)
#define ADD_TAG_ID       (TAG_VALUE_ID     + 1)
#define DELETE_TAG_ID    (ADD_TAG_ID       + 1)

#define ENTITY_LABEL_LENGTH 16

typedef struct EntityBrowser_tag {
    struct Window *window;
    struct Gadget *gadgets;
    struct Gadget *addEntityGadget;
    struct Gadget *removeEntityGadget;
    struct Gadget *entityListGadget;
    struct Gadget *rowGadget;
    struct Gadget *colGadget;
    struct Gadget *VRAMSlotGadget;
    struct Gadget *addTagGadget;
    struct Gadget *deleteTagGadget;
    struct Gadget *chooseEntityGadget;
    struct Gadget *tagListGadget;
    struct Gadget *tagAliasGadget;
    struct Gadget *tagIdGadget;
    struct Gadget *tagValueGadget;
    char *title;
    int closed;
    int selectedEntity;
    int selectedTag;
    struct List entityLabels;
    struct Node *entityNodes;
    char (*entityStrings)[ENTITY_LABEL_LENGTH];
    struct List tagLabels;
    struct Node *tagNodes;
    char (*tagStrings)[TAG_ALIAS_LENGTH + 4];
} EntityBrowser;

void initEntityBrowserScreen(void);
void initEntityBrowserVi(void);

EntityBrowser *newEntityBrowser(char *title, Entity *entities, int entityCnt);
void freeEntityBrowser(EntityBrowser*);

void entityBrowserSelectEntity(EntityBrowser*, int entityNum, Entity*);
void entityBrowserDeselectEntity(EntityBrowser*);

void entityBrowserFreeEntityLabels(EntityBrowser*);
int entityBrowserSetEntities(EntityBrowser*, Entity *entities, int entityCnt);

void entityBrowserSelectTag(EntityBrowser*, int tagNum, Frac_tag*);

void entityBrowserFreeTagLabels(EntityBrowser*);
int entityBrowserSetTags(EntityBrowser*, Frac_tag *tags, int tagCnt);

#endif
