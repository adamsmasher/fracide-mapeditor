#ifndef ENTITY_BROWSER_H
#define ENTITY_BROWSER_H

#include <proto/intuition.h>

#define ENTITY_LIST_ID   0
#define ADD_ENTITY_ID    (ENTITY_LIST_ID   + 1)
#define REMOVE_ENTITY_ID (ADD_ENTITY_ID    + 1)
#define THIS_ENTITY_ID   (REMOVE_ENTITY_ID + 1)
#define CHOOSE_ENTITY_ID (THIS_ENTITY_ID   + 1)
#define ENTITY_ROW_ID    (CHOOSE_ENTITY_ID + 1)
#define ENTITY_COL_ID    (ENTITY_ROW_ID    + 1)
#define VRAM_SLOT_ID     (ENTITY_COL_ID    + 1)

typedef struct EntityBrowser_tag {
    struct Window *window;
    struct Gadget *gadgets;
    char *title;
    int closed;
} EntityBrowser;

void initEntityBrowserScreen(void);
void initEntityBrowserVi(void);

EntityBrowser *newEntityBrowser(char *title);
void freeEntityBrowser(EntityBrowser*);

#endif