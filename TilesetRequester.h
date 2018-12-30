#ifndef TILESET_REQUESTER_H
#define TILESET_REQUESTER_H

#include <intuition/intuition.h>

#include "framework/Window.h"

FrameworkWindow *newTilesetRequester(FrameworkWindow *parent, const char *title);

BOOL isTilesetRequester(FrameworkWindow*);

void tilesetRequesterRefresh(FrameworkWindow*);

#endif
