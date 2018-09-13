#ifndef MAP_EDITOR_DATA_H
#define MAP_EDITOR_DATA_H

#include "framework/Window.h"

#include "Entity.h"
#include "Map.h"
#include "MapEditorGadgets.h"
#include "SongRequester.h"

typedef struct MapEditorData_tag MapEditorData;

MapEditorData *newMapEditorData(void);
void initMapEditorData(MapEditorData*, FrameworkWindow*, Map*);
void freeMapEditorData(MapEditorData*);

struct Image *mapEditorDataGetPaletteImages(MapEditorData*);
struct Image *mapEditorDataGetMapImages(MapEditorData*);

const Map *mapEditorDataGetMap(MapEditorData*);

const MapEditorGadgets *mapEditorDataGetGadgets(MapEditorData*);

BOOL mapEditorDataIsSaved(MapEditorData*);
BOOL mapEditorDataSaveMap(MapEditorData*);
BOOL mapEditorDataSaveMapAs(MapEditorData*, int mapNum);

const char *mapEditorDataGetTitle(MapEditorData*);

BOOL mapEditorDataHasMapNum(MapEditorData*);
/* results are undefined if map editor does not have a map num */
UWORD mapEditorDataGetMapNum(MapEditorData*);
void mapEditorDataSetMapNum(MapEditorData*, UWORD mapNum);

const char *mapEditorDataGetMapName(MapEditorData*);
void mapEditorDataSetMapName(MapEditorData*, const char *mapName);

BOOL mapEditorDataHasSongRequester(MapEditorData*);
SongRequester *mapEditorDataGetSongRequester(MapEditorData*);
void mapEditorDataSetSongRequester(MapEditorData*, SongRequester*);

BOOL mapEditorDataHasEntityBrowser(MapEditorData*);
FrameworkWindow *mapEditorDataGetEntityBrowser(MapEditorData*);
void mapEditorDataSetEntityBrowser(MapEditorData*, FrameworkWindow*);

BOOL mapEditorDataHasTilesetRequester(MapEditorData*);
FrameworkWindow *mapEditorDataGetTilesetRequester(MapEditorData*);
void mapEditorDataSetTilesetRequester(MapEditorData*, FrameworkWindow*);

void mapEditorDataAddNewEntity(MapEditorData*);
void mapEditorDataRemoveEntity(MapEditorData*, UWORD entityNum);

UWORD mapEditorDataGetEntityCount(MapEditorData*);

const Entity *mapEditorDataGetEntity(MapEditorData*, UWORD entityNum);

/* TODO: delete the getters, just use a const pointer lol */
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

BOOL mapEditorDataHasSong(MapEditorData*);
void mapEditorDataClearSong(MapEditorData*);
void mapEditorDataSetSong(MapEditorData*, UWORD songNum);
UWORD mapEditorDataGetSong(MapEditorData*);

void mapEditorDataClearTileset(MapEditorData*);
BOOL mapEditorDataHasTileset(MapEditorData*);
void mapEditorDataSetTileset(MapEditorData*, UWORD tilesetNum);

void mapEditorDataSetTileTo(MapEditorData*, UBYTE row, UBYTE col, UBYTE to);

struct Image *mapEditorDataGetMapImage(MapEditorData*, UBYTE tile);
struct Image *mapEditorDataGetPaletteImage(MapEditorData*, unsigned int tile);

BOOL mapEditorDataHasSelected(MapEditorData*);
unsigned int mapEditorDataGetSelected(MapEditorData*);
void mapEditorDataSetSelected(MapEditorData*, unsigned int selected);

#endif
