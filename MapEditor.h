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
UBYTE mapEditorGetEntityRow(MapEditorData*, UWORD entityNum);
UBYTE mapEditorGetEntityCol(MapEditorData*, UWORD entityNum);
UBYTE mapEditorGetEntityVRAMSlot(MapEditorData*, UWORD entityNum);
void mapEditorSetEntityRow(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE row);
void mapEditorSetEntityCol(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE col);
void mapEditorSetEntityVRAMSlot(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE vramSlot);

void mapEditorEntityAddNewTag(FrameworkWindow *mapEditor, UWORD entityNum);
void mapEditorEntityDeleteTag(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum);
int mapEditorEntityGetTagCount(MapEditorData*, UWORD entityNum);
const char *mapEditorEntityGetTagAlias(MapEditorData*, UWORD entityNum, int tagNum);
UBYTE mapEditorEntityGetTagId(MapEditorData*, UWORD entityNum, int tagNum);
void mapEditorEntitySetTagId(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagId);
UBYTE mapEditorEntityGetTagValue(MapEditorData*, UWORD entityNum, int tagNum);
void mapEditorEntitySetTagAlias(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, const char *newTagAlias);
void mapEditorEntitySetTagValue(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagValue);

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor);

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum);
void mapEditorRedrawTile(FrameworkWindow *mapEditor, int row, int col);

#endif
