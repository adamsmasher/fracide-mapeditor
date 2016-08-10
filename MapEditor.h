#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "TilesetRequester.h"

#include <intuition/intuition.h>

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)
#define TILESET_SCROLL_ID  (CHOOSE_TILESET_ID  + 1)

#define TILESET_PALETTE_TILES_ACROSS 4
#define TILESET_PALETTE_TILES_HIGH   8

typedef struct MapEditorTag {
	struct MapEditorTag *next;
	struct MapEditorTag *prev;
	struct Window *window;
	struct Gadget *gadgets;
	struct Gadget *tilesetNameGadget;
	int closed;
	TilesetRequester *tilesetRequester;
	struct Image images[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
	UWORD *imageData;
} MapEditor;

void initMapEditorScreen(void);
void initMapEditorVi(void);

MapEditor *newMapEditor(void);
void closeMapEditor(MapEditor*);

void refreshMapEditor(MapEditor*);

void attachTilesetRequesterToMapEditor(MapEditor*, TilesetRequester*);

void mapEditorSetTileset(MapEditor*, UWORD);

#endif
