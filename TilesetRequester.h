#ifndef TILESET_REQUESTER_H
#define TILESET_REQUESTER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#define TILESET_LIST_ID 0

typedef struct TilesetRequesterDataTag {
    struct Gadget *gadgets;
    struct Gadget *tilesetList;
    char          *title;
} TilesetRequesterData;

FrameworkWindow *newTilesetRequester(char *title, FrameworkWindow *parent);
void closeTilesetRequester(FrameworkWindow*);

BOOL isTilesetRequesterWindow(FrameworkWindow*);

void refreshTilesetRequesterList(FrameworkWindow*);
void resizeTilesetRequester(FrameworkWindow*);

#endif
