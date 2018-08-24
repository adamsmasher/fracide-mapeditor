#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#include "EntityBrowser.h"
#include "SongRequester.h"
#include "TilesetRequester.h"
#include "Map.h"

FrameworkWindow *newMapEditorNewMap(FrameworkWindow *parent);
FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map*, int mapNum);

BOOL isMapEditor(FrameworkWindow*);

void mapEditorUpdateMapName(FrameworkWindow*);
void mapEditorChooseTilesetClicked(FrameworkWindow*);
void mapEditorChangeSongClicked(FrameworkWindow*);
void mapEditorClearSongClicked(FrameworkWindow*);
void mapEditorMapLeftClicked(FrameworkWindow*);
void mapEditorMapRightClicked(FrameworkWindow*);
void mapEditorMapUpClicked(FrameworkWindow*);
void mapEditorMapDownClicked(FrameworkWindow*);
void mapEditorEntitiesClicked(FrameworkWindow*);

void mapEditorSetMapNum(FrameworkWindow *mapEditor, UWORD mapNum);

void mapEditorSetTileset(FrameworkWindow *mapEditor, UWORD tilesetNum);
void mapEditorRefreshTileset(FrameworkWindow *mapEditor);
void mapEditorUpdateTileDisplays(FrameworkWindow *mapEditor);

void mapEditorSetSong(FrameworkWindow *mapEditor, UWORD songNum);
void mapEditorRefreshSong(FrameworkWindow *mapEditor);

void mapEditorAddNewEntity(FrameworkWindow *mapEditor);
void mapEditorRemoveEntity(FrameworkWindow *mapEditor, UWORD entityNum);
void mapEditorSetEntityRow(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE row);
void mapEditorSetEntityCol(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE col);
void mapEditorSetEntityVRAMSlot(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE vramSlot);

void mapEditorEntityAddNewTag(FrameworkWindow *mapEditor, UWORD entityNum);
void mapEditorEntityDeleteTag(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum);
void mapEditorEntitySetTagAlias(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, const char *newTagAlias);
void mapEditorEntitySetTagId(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagId);
void mapEditorEntitySetTagValue(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagValue);

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor);

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum);
void mapEditorRedrawTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col);

#endif
