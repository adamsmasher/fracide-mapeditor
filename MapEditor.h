#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "TilesetRequester.h"
#include "Map.h"

#include <intuition/intuition.h>

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)
#define TILESET_SCROLL_ID  (CHOOSE_TILESET_ID  + 1)
#define MAP_NAME_ID        (TILESET_SCROLL_ID  + 1)

#define TILESET_PALETTE_TILES_ACROSS 4
#define TILESET_PALETTE_TILES_HIGH   8

#define MAP_TILES_ACROSS 10
#define MAP_TILES_HIGH    9

typedef struct MapEditorTag {
    struct MapEditorTag *next;
    struct MapEditorTag *prev;

    struct Window *window;
    struct Gadget *gadgets;

    Map *map;
    int mapNum;

    struct Gadget *tilesetNameGadget;
    struct Gadget *mapNameGadget;

    int closed;
    int saved;

    TilesetRequester *tilesetRequester;

    struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
    struct Image mapImages[MAP_TILES_ACROSS * MAP_TILES_HIGH];
    UWORD *imageData;

    int selected;
} MapEditor;

void initMapEditorScreen(void);
void initMapEditorVi(void);

struct Menu *initMapEditorMenu(void);
void freeMapEditorMenu(void);

MapEditor *newMapEditorNewMap(void);
MapEditor *newMapEditorWithMap(Map*, int mapNum);
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
