#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#include "EntityBrowser.h"
#include "SongRequester.h"
#include "TilesetRequester.h"
#include "Map.h"

/* constructors */
FrameworkWindow *newMapEditorNewMap(FrameworkWindow *parent);
FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map*, UWORD mapNum);

/* properties */
BOOL isMapEditor(FrameworkWindow*);

/* event handlers */
/* (menus) */
void mapEditorNewMap(FrameworkWindow*);
void mapEditorOpenMap(FrameworkWindow*);
BOOL mapEditorSaveMap(FrameworkWindow*);
BOOL mapEditorSaveMapAs(FrameworkWindow*);
void mapEditorRevertMap(FrameworkWindow*);

/* (buttons) */
void mapEditorChooseTilesetClicked(FrameworkWindow*);
void mapEditorChangeSongClicked(FrameworkWindow*);
void mapEditorClearSongClicked(FrameworkWindow*);
void mapEditorMapLeftClicked(FrameworkWindow*);
void mapEditorMapRightClicked(FrameworkWindow*);
void mapEditorMapUpClicked(FrameworkWindow*);
void mapEditorMapDownClicked(FrameworkWindow*);
void mapEditorEntitiesClicked(FrameworkWindow*);

/* (misc) */
void mapEditorUpdateMapName(FrameworkWindow*);
BOOL mapEditorEnsureSaved(FrameworkWindow*);

/* children */
FrameworkWindow *mapEditorGetEntityBrowser(FrameworkWindow*);
FrameworkWindow *mapEditorGetTilesetRequester(FrameworkWindow*);

/* refreshing */
void mapEditorRefreshEntityBrowser(FrameworkWindow*);
void mapEditorRefreshMapName(FrameworkWindow*);
void mapEditorRefreshNavigationButtons(FrameworkWindow*);
void mapEditorRefreshRevertMap(FrameworkWindow*);
void mapEditorRefreshSong(FrameworkWindow*);
void mapEditorRefreshSongDisplays(FrameworkWindow*);
void mapEditorRefreshSelected(FrameworkWindow*);
void mapEditorRefreshSelectedFrom(FrameworkWindow*, unsigned int selected);
void mapEditorRefreshTitle(FrameworkWindow*);
void mapEditorRefreshTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col);
void mapEditorRefreshTileDisplays(FrameworkWindow*);
void mapEditorRefreshTilesetName(FrameworkWindow*);

#endif
