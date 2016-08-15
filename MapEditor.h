#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "TilesetRequester.h"

#include <intuition/intuition.h>

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)
#define TILESET_SCROLL_ID  (CHOOSE_TILESET_ID  + 1)

#define TILESET_PALETTE_TILES_ACROSS 4
#define TILESET_PALETTE_TILES_HIGH   8

#define MAP_TILES_ACROSS 10
#define MAP_TILES_HIGH    9

typedef struct MapEditorTag {
	struct MapEditorTag *next;
	struct MapEditorTag *prev;

	struct Window *window;
	struct Gadget *gadgets;
	struct Gadget *tilesetNameGadget;

	int closed;

	TilesetRequester *tilesetRequester;

	UWORD tilesetNum;

	struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
	struct Image mapImages[MAP_TILES_ACROSS * MAP_TILES_HIGH];
	UWORD *imageData;

	int selected;
} MapEditor;

void initMapEditorScreen(void);
void initMapEditorVi(void);

MapEditor *newMapEditor(void);
void closeMapEditor(MapEditor*);

void refreshMapEditor(MapEditor*);

void attachTilesetRequesterToMapEditor(MapEditor*, TilesetRequester*);

void mapEditorSetTileset(MapEditor*, UWORD);

int mapEditorClickInPalette(WORD x, WORD y);
unsigned int mapEditorGetPaletteTileClicked(WORD x, WORD y);
void mapEditorSetSelected(MapEditor*, unsigned int);

int mapEditorClickInMap(WORD x, WORD y);
unsigned int mapEditorGetMapTileClicked(WORD x, WORD y);
void mapEditorSetTile(MapEditor*, unsigned int);

#endif
