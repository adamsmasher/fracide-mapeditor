#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <intuition/intuition.h>

#include "framework/Window.h"

#include "EntityBrowser.h"
#include "SongRequester.h"
#include "TilesetRequester.h"
#include "Map.h"

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)
#define TILESET_SCROLL_ID  (CHOOSE_TILESET_ID  + 1)
#define MAP_NAME_ID        (TILESET_SCROLL_ID  + 1)
#define SONG_NAME_LABEL_ID (MAP_NAME_ID        + 1)
#define SONG_CHANGE_ID     (SONG_NAME_LABEL_ID + 1)
#define SONG_CLEAR_ID      (SONG_CHANGE_ID     + 1)
#define MAP_LEFT_ID        (SONG_CLEAR_ID      + 1)
#define MAP_RIGHT_ID       (MAP_LEFT_ID        + 1)
#define MAP_UP_ID          (MAP_RIGHT_ID       + 1)
#define MAP_DOWN_ID        (MAP_UP_ID          + 1)
#define ENTITIES_ID        (MAP_DOWN_ID        + 1)

#define TILESET_PALETTE_TILES_ACROSS 4
#define TILESET_PALETTE_TILES_HIGH   8

#define MAP_TILES_ACROSS 10
#define MAP_TILES_HIGH    9

typedef struct MapEditorData_tag {
  struct Gadget *gadgets;

  Map *map;
  int mapNum;

  struct Gadget *tilesetNameGadget;
  struct Gadget *mapNameGadget;
  struct Gadget *songNameGadget;
  struct Gadget *leftGadget;
  struct Gadget *rightGadget;
  struct Gadget *upGadget;
  struct Gadget *downGadget;

  BOOL saved;

  TilesetRequester *tilesetRequester;
  SongRequester    *songRequester;
  EntityBrowser    *entityBrowser;

  struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
  struct Image mapImages[MAP_TILES_ACROSS * MAP_TILES_HIGH];
  UWORD *imageData;

  int selected;

  char title[16];
} MapEditorData;

FrameworkWindow *newMapEditorNewMap(void);
FrameworkWindow *newMapEditorWithMap(Map*, int mapNum);

BOOL isMapEditorWindow(FrameworkWindow*);

void mapEditorSetMapNum(FrameworkWindow *mapEditor, UWORD mapNum);

void mapEditorSetTileset(FrameworkWindow *mapEditor, UWORD tilesetNum);
void mapEditorRefreshTileset(FrameworkWindow *mapEditor);
void mapEditorUpdateTileDisplays(FrameworkWindow *mapEditor);

void mapEditorSetSong(FrameworkWindow *mapEditor, UWORD songNum);
void mapEditorRefreshSong(FrameworkWindow *mapEditor);

typedef enum SaveStatus_tag {
  UNSAVED,
  SAVED
} SaveStatus;

void mapEditorSetSaveStatus(FrameworkWindow *mapEditor, SaveStatus);
BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor);

void mapEditorDrawEntity(FrameworkWindow *mapEditor, Entity*, int entityNum);
void mapEditorRedrawTile(FrameworkWindow *mapEditor, int row, int col);

void enableMapRevert(FrameworkWindow *mapEditor);
void disableMapRevert(FrameworkWindow *mapEditor);

#endif
