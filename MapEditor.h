#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include "SongNames.h"
#include "TilesetRequester.h"
#include "Map.h"

#include <intuition/intuition.h>

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)
#define TILESET_SCROLL_ID  (CHOOSE_TILESET_ID  + 1)
#define MAP_NAME_ID        (TILESET_SCROLL_ID  + 1)
#define SONG_NAME_LABEL_ID (MAP_NAME_ID        + 1)
#define SONG_CHANGE_ID     (SONG_NAME_LABEL_ID + 1)
#define SONG_CLEAR_ID      (SONG_CHANGE_ID     + 1)

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
    struct Gadget *songNameGadget;

    int closed;
    int saved;

    TilesetRequester *tilesetRequester;
    SongRequester    *songRequester;

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
void attachSongRequesterToMapEditor(MapEditor*, SongRequester*);

void mapEditorSetTileset(MapEditor*, UWORD);
void mapEditorSetSong(MapEditor*, UWORD);
void mapEditorClearSong(MapEditor*);

int mapEditorClickInPalette(WORD x, WORD y);
unsigned int mapEditorGetPaletteTileClicked(WORD x, WORD y);
void mapEditorSetSelected(MapEditor*, unsigned int);

int mapEditorClickInMap(WORD x, WORD y);
unsigned int mapEditorGetMapTileClicked(WORD x, WORD y);
void mapEditorSetTile(MapEditor*, unsigned int);
void updateMapEditorMapName(MapEditor*);

#endif
