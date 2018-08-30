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

void mapEditorNewMap(FrameworkWindow*);
void mapEditorOpenMap(FrameworkWindow*);
BOOL mapEditorSaveMap(FrameworkWindow*);
BOOL mapEditorSaveMapAs(FrameworkWindow*);
void mapEditorRevertMap(FrameworkWindow*);

void mapEditorRefreshRevertMap(FrameworkWindow*);
void mapEditorRefreshTitle(FrameworkWindow*);

void mapEditorUpdateMapName(FrameworkWindow*);
void mapEditorChooseTilesetClicked(FrameworkWindow*);
void mapEditorChangeSongClicked(FrameworkWindow*);
void mapEditorClearSongClicked(FrameworkWindow*);
void mapEditorMapLeftClicked(FrameworkWindow*);
void mapEditorMapRightClicked(FrameworkWindow*);
void mapEditorMapUpClicked(FrameworkWindow*);
void mapEditorMapDownClicked(FrameworkWindow*);
void mapEditorEntitiesClicked(FrameworkWindow*);

void mapEditorUpdateTileDisplays(FrameworkWindow *mapEditor);

void mapEditorRefreshSong(FrameworkWindow *mapEditor);

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor);

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum);
void mapEditorRedrawTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col);

#endif
