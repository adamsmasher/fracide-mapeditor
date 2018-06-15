#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#include "EntityBrowser.h"
#include "SongRequester.h"
#include "TilesetRequester.h"
#include "Map.h"

#define TILESET_PALETTE_TILES_ACROSS 4
#define TILESET_PALETTE_TILES_HIGH   8

#define MAP_TILES_ACROSS 10
#define MAP_TILES_HIGH    9

typedef struct MapEditorData_tag MapEditorData;

FrameworkWindow *newMapEditorNewMap(FrameworkWindow *parent);
FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map*, int mapNum);

BOOL isMapEditorWindow(FrameworkWindow*);

void mapEditorSetMapNum(FrameworkWindow *mapEditor, UWORD mapNum);

void mapEditorSetTileset(FrameworkWindow *mapEditor, UWORD tilesetNum);
void mapEditorRefreshTileset(FrameworkWindow *mapEditor);
void mapEditorUpdateTileDisplays(FrameworkWindow *mapEditor);

void mapEditorSetSong(FrameworkWindow *mapEditor, UWORD songNum);
void mapEditorRefreshSong(FrameworkWindow *mapEditor);

UWORD mapEditorEntityCount(MapEditorData*);
void mapEditorAddNewEntity(FrameworkWindow *mapEditor);
void mapEditorRemoveEntity(FrameworkWindow *mapEditor, UWORD entityNum);
void mapEditorSetEntityRow(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE row);
void mapEditorSetEntityCol(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE col);
void mapEditorSetEntityVRAMSlot(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE vramSlot);

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor);

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum);
void mapEditorRedrawTile(FrameworkWindow *mapEditor, int row, int col);

#endif
