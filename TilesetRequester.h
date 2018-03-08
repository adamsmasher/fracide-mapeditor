#ifndef TILESET_REQUESTER_H
#define TILESET_REQUESTER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#define TILESET_LIST_ID 0

typedef struct TilesetRequesterTag {
    FrameworkWindow *window;
    struct Gadget *gadgets;
    struct Gadget *tilesetList;
    int           closed;
    char          *title;
} TilesetRequester;

TilesetRequester *newTilesetRequester(char *title);
void closeTilesetRequester(TilesetRequester*);

BOOL isTilesetRequesterWindow(FrameworkWindow*);

void refreshTilesetRequesterList(TilesetRequester*);
void resizeTilesetRequester(TilesetRequester*);

#endif
