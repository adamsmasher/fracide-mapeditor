#ifndef MAP_EDITOR_DATA_H
#define MAP_EDITOR_DATA_H

#include "framework/Window.h"

#include "Map.h"
#include "MapEditorConstants.h"
#include "MapEditorGadgets.h"
#include "SaveStatus.h"
#include "SongRequester.h"

typedef struct MapEditorData_tag {
  FrameworkWindow *window;

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

MapEditorData *newMapEditorData(void);
void freeMapEditorData(MapEditorData*);

/* TODO: there should be a way for new to do this */
void mapEditorDataInitImages(MapEditorData*);
void *mapEditorDataGetImageDataForTile(MapEditorData*, UBYTE tile);

/* save status is set automatically in response to changes, but
   sometimes may need to be set manually in response e.g. to saving */
void mapEditorDataSetSaveStatus(MapEditorData*, SaveStatus);

UWORD mapEditorDataGetMapNum(MapEditorData*);
void mapEditorDataSetMapNum(MapEditorData*, UWORD mapNum);

void mapEditorDataSetMapName(MapEditorData*, const char*);

BOOL mapEditorDataHasSongRequester(MapEditorData*);
void mapEditorDataSetSongRequester(MapEditorData*, SongRequester*);

BOOL mapEditorDataHasEntityBrowser(MapEditorData*);

void mapEditorDataAddNewEntity(MapEditorData*);
void mapEditorDataRemoveEntity(MapEditorData*, UWORD entityNum);

UWORD mapEditorDataGetEntityCount(MapEditorData*);

UBYTE mapEditorDataGetEntityRow(MapEditorData*, UWORD entityNum);
void mapEditorDataSetEntityRow(MapEditorData*, UWORD entityNum, UBYTE row);
UBYTE mapEditorDataGetEntityCol(MapEditorData*, UWORD entityNum);
void mapEditorDataSetEntityCol(MapEditorData*, UWORD entityNum, UBYTE col);
UBYTE mapEditorDataGetEntityVRAMSlot(MapEditorData*, UWORD entityNum);
void mapEditorDataSetEntityVRAMSlot(MapEditorData*, UWORD entityNum, UBYTE vramSlot);

int mapEditorDataEntityGetTagCount(MapEditorData*, UWORD entityNum);

void mapEditorDataEntityAddNewTag(MapEditorData*, UWORD entityNum);
void mapEditorDataEntityDeleteTag(MapEditorData*, UWORD entityNum, int tagNum);

const char *mapEditorDataEntityGetTagAlias(MapEditorData*, UWORD entityNum, int tagNum);
void mapEditorDataEntitySetTagAlias(MapEditorData*, UWORD entityNum, int tagNum, const char *newTagAlias);
UBYTE mapEditorDataEntityGetTagId(MapEditorData*, UWORD entityNum, int tagNum);
void mapEditorDataEntitySetTagId(MapEditorData*, UWORD entityNum, int tagNum, UBYTE newTagId);
UBYTE mapEditorDataEntityGetTagValue(MapEditorData*, UWORD entityNum, int tagNum);
void mapEditorDataEntitySetTagValue(MapEditorData*, UWORD entityNum, int tagNum, UBYTE newTagValue);

void mapEditorDataClearSong(MapEditorData*);
void mapEditorDataSetSong(MapEditorData*, UWORD songNum);

void mapEditorDataClearTileset(MapEditorData*);
void mapEditorDataSetTileset(MapEditorData*, UWORD tilesetNum);

void mapEditorDataSetTileTo(MapEditorData*, UBYTE row, UBYTE col, UBYTE to);

#endif
