#ifndef MAP_EDITOR_DATA_H
#define MAP_EDITOR_DATA_H

#include "framework/Window.h"

#include "Map.h"
#include "MapEditorConstants.h"
#include "MapEditorGadgets.h"
#include "SaveStatus.h"
#include "SongRequester.h"

typedef struct MapEditorData_tag {
  Map *map;
  UWORD mapNum;

  MapEditorGadgets gadgets;

  SaveStatus saveStatus;

  FrameworkWindow *tilesetRequester;
  SongRequester   *songRequester;
  FrameworkWindow *entityBrowser;

  struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
  struct Image mapImages[MAP_TILES_WIDE * MAP_TILES_HIGH];
  UWORD *imageData;

  int selected;

  char title[16];
} MapEditorData;

void mapEditorDataInitPaletteImages(MapEditorData*);
void mapEditorDataInitMapImages(MapEditorData*);
void mapEditorDataInitImages(MapEditorData*);
void *mapEditorDataGetImageDataForTile(MapEditorData*, UBYTE tile);

/* TODO: this should happen automatically on state changes */
void mapEditorDataSetSaveStatus(MapEditorData*, SaveStatus);

UWORD mapEditorDataGetMapNum(MapEditorData*);

BOOL mapEditorDataHasSongRequester(MapEditorData*);
void mapEditorDataSetSongRequester(MapEditorData*, SongRequester*);

BOOL mapEditorDataHasEntityBrowser(MapEditorData*);

UWORD mapEditorDataGetEntityCount(MapEditorData*);

UBYTE mapEditorDataGetEntityRow(MapEditorData*, UWORD entityNum);
UBYTE mapEditorDataGetEntityCol(MapEditorData*, UWORD entityNum);
UBYTE mapEditorDataGetEntityVRAMSlot(MapEditorData*, UWORD entityNum);

int mapEditorDataEntityGetTagCount(MapEditorData*, UWORD entityNum);
const char *mapEditorDataEntityGetTagAlias(MapEditorData*, UWORD entityNum, int tagNum);
UBYTE mapEditorDataEntityGetTagId(MapEditorData*, UWORD entityNum, int tagNum);
UBYTE mapEditorDataEntityGetTagValue(MapEditorData*, UWORD entityNum, int tagNum);

#endif
