#ifndef ENTITY_BROWSER_H
#define ENTITY_BROWSER_H

#include "framework/Window.h"

#include "Map.h"

FrameworkWindow *newEntityBrowserWithMapNum(FrameworkWindow *parent, const Map*, UWORD mapNum);

BOOL isEntityBrowser(FrameworkWindow*);

/* sets what kind of entity the current entity is */
void entityBrowserSetEntityNum(FrameworkWindow *entityBrowser, UBYTE entityKind);

void entityBrowserRefresh(FrameworkWindow *entityBrowser);

#endif
