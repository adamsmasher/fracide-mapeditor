#ifndef ENTITY_BROWSER_H
#define ENTITY_BROWSER_H

#include <proto/intuition.h>

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