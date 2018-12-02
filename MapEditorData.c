#include "MapEditorData.h"

#include <exec/exec.h>
#include <proto/exec.h>

#include <graphics/gfx.h>
#include <graphics/scale.h>
#include <proto/graphics.h>

#include <stdlib.h>
#include <string.h>

#include "MapEditor.h"
#include "MapEditorConstants.h"
#include "ProjectWindowData.h"
#include "TilesetPackage.h"

#define IMAGE_DATA_SIZE (TILES_PER_SET * 256)

struct MapEditorData_tag {
  FrameworkWindow *window;

  Map *map;
  UWORD mapNum;

  BOOL saved;

  /* TODO: get ridda me */
  FrameworkWindow *tilesetRequester;
  FrameworkWindow *entityBrowser;

  struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
  struct Image mapImages[MAP_TILES_WIDE * MAP_TILES_HIGH];
  UWORD *imageData;

  int selected;

  char title[16];
};

static void mapEditorDataInitPaletteImages(MapEditorData *data) {
  int top, left, row, col;
  struct Image *i = data->paletteImages;
  UWORD *imageData = data->imageData;

  top = 0;
  for(row = 0; row < TILESET_PALETTE_TILES_HIGH; row++) {
    left = 0;
    for(col = 0; col < TILESET_PALETTE_TILES_ACROSS; col++) {
      i->LeftEdge = left;
      i->TopEdge = top;
      i->Width = 32;
      i->Height = 32;
      i->Depth = 2;
      i->ImageData = imageData;
      i->PlanePick = 0x03;
      i->PlaneOnOff = 0;
      i->NextImage = i + 1;

      i++;
      left += 32;
      imageData += 128;
    }
    top += 32;
  }
  data->paletteImages[31].NextImage = NULL;
}

static void mapEditorDataInitMapImages(MapEditorData *data) {
  int top, left, row, col;
  struct Image *i = data->mapImages;
  UWORD *imageData = data->imageData;

  top = 0;
  for(row = 0; row < MAP_TILES_HIGH; row++) {
    left = 0;
    for(col = 0; col < MAP_TILES_WIDE; col++) {
      i->LeftEdge = left;
      i->TopEdge = top;
      i->Width = 32;
      i->Height = 32;
      i->Depth = 2;
      i->ImageData = imageData;
      i->PlanePick = 0x03;
      i->PlaneOnOff = 0;
      i->NextImage = i + 1;

      i++;
      left += 32;
    }
    top += 32;
  }
  data->mapImages[89].NextImage = NULL;
}

static void *mapEditorDataGetImageDataForTile(MapEditorData *data, UBYTE tile) {
  return data->imageData + (tile << 7);
}

static void mapEditorDataInitImages(MapEditorData *data) {
  Map *map = data->map;
  int i;
  for(i = 0; i < MAP_TILES_HIGH * MAP_TILES_WIDE; i++) {
    data->mapImages[i].ImageData = mapEditorDataGetImageDataForTile(data, map->tiles[i]);
  }
}

MapEditorData *newMapEditorData(void) {
  MapEditorData *data = malloc(sizeof(MapEditorData));
  if(!data) {
    fprintf(stderr, "newMapEditorData: failed to allocate MapEditorData\n");
    goto error;
  }

  data->imageData = AllocMem(IMAGE_DATA_SIZE, MEMF_CHIP);
  if(!data->imageData) {
    fprintf(stderr, "newMapEditorData: failed to allocate image data\n");
    goto error_freeData;
  }
  mapEditorDataInitPaletteImages(data);
  mapEditorDataInitMapImages(data);

  data->window           = NULL;
  data->map              = NULL;
  data->mapNum           = 0;
  data->saved            = TRUE;
  data->tilesetRequester = NULL;
  data->entityBrowser    = NULL;
  data->selected         = -1;
  data->title[0]         = '\0';

  return data;
error_freeData:
  free(data);
error:
  return NULL;
}

static void mapEditorDataUpdateTitle(MapEditorData *data) {
  char unsaved = data->saved ? '\0' : '*';
  if(data->mapNum) {
    sprintf(data->title, "Map %d%c", data->mapNum - 1, unsaved);
  } else {
    sprintf(data->title, "Map Editor%c", unsaved);
  }
  mapEditorRefreshTitle(data->window);
}

void initMapEditorData(MapEditorData *data, FrameworkWindow *window, Map *map) {
  window->data = data;
  data->window = window;
  data->map = map;

  mapEditorDataUpdateTitle(data);

  if(mapEditorDataHasTileset(data)) {
    mapEditorDataInitImages(data);
  }

  mapEditorRefreshMapName(data->window);

  if(map->songNum) {
    mapEditorDataSetSong(data, map->songNum - 1);
  }
}

void freeMapEditorData(MapEditorData* data) {
  if(data->map) {
    free(data->map);
  }
  FreeMem(data->imageData, IMAGE_DATA_SIZE);
  free(data);
}

const Map *mapEditorDataGetMap(MapEditorData *data) {
  return data->map;
}

struct Image *mapEditorDataGetMapImages(MapEditorData *data) {
  return data->mapImages;
}

struct Image *mapEditorDataGetPaletteImages(MapEditorData *data) {
  return data->paletteImages;
}

const char *mapEditorDataGetTitle(MapEditorData *data) {
  return data->title;
}

BOOL mapEditorDataIsSaved(MapEditorData *data) {
  return data->saved;
}

static void mapEditorDataSetSaved(MapEditorData *data, BOOL saved) {
  if(saved != data->saved) {
    data->saved = saved;
    mapEditorRefreshRevertMap(data->window);
    mapEditorDataUpdateTitle(data);
  }
}

BOOL mapEditorDataSaveMapAs(MapEditorData *data, UWORD mapNum) {
  ProjectWindowData *projectData = data->window->parent->data;

  if(!projectDataSaveMap(projectData, data->map, mapNum)) {
    fprintf(stderr, "mapEditorDataSaveMapAs: failed to save map\n");
    goto error;
  }

  mapEditorDataSetMapNum(data, mapNum);
  mapEditorDataSetSaved(data, TRUE);
  return TRUE;

error:
  return FALSE;
}

BOOL mapEditorDataSaveMap(MapEditorData *data) {
  ProjectWindowData *projectData = data->window->parent->data;

  if(!mapEditorDataHasMapNum(data)) {
    fprintf(stderr, "mapEditorDataSaveMap: assertion error: map must have map num\n");
    goto error;
  }

  if(!projectDataSaveMap(projectData, data->map, mapEditorDataGetMapNum(data))) {
    fprintf(stderr, "mapEditorDataSaveMap: failed to save map\n");
    goto error;
  }

  mapEditorDataSetSaved(data, TRUE);
  return TRUE;
error:
  return FALSE;
}

BOOL mapEditorDataHasMapNum(MapEditorData *data) {
  return (BOOL)(data->mapNum != 0);
}

/* results are undefined if the map editor does not have a map */
UWORD mapEditorDataGetMapNum(MapEditorData *data) {
  return (UWORD)(data->mapNum - 1);
}

void mapEditorDataSetMapNum(MapEditorData *data, UWORD mapNum) {
  data->mapNum = mapNum + 1;

  mapEditorDataUpdateTitle(data);

  mapEditorRefreshNavigationButtons(data->window);
}

const char *mapEditorDataGetMapName(MapEditorData *data) {
  return data->map->name;
}

void mapEditorDataSetMapName(MapEditorData *data, const char *mapName) {
  strcpy(data->map->name, mapName);
  mapEditorDataSetSaved(data, FALSE);
}

BOOL mapEditorDataHasTilesetRequester(MapEditorData *data) {
  return (BOOL)(data->tilesetRequester != NULL);
}

void mapEditorDataSetTilesetRequester(MapEditorData *data, FrameworkWindow *tilesetRequester) {
  data->tilesetRequester = tilesetRequester;
}

FrameworkWindow *mapEditorDataGetTilesetRequester(MapEditorData *data) {
  return data->tilesetRequester;
}

BOOL mapEditorDataHasEntityBrowser(MapEditorData *data) {
  return (BOOL)(data->entityBrowser != NULL);
}

FrameworkWindow* mapEditorDataGetEntityBrowser(MapEditorData *data) {
  return data->entityBrowser;
}

void mapEditorDataSetEntityBrowser(MapEditorData *data, FrameworkWindow *entityBrowser) {
  data->entityBrowser = entityBrowser;
}

void mapEditorDataAddNewEntity(MapEditorData *data) {
  Entity *newEntity = mapAddNewEntity(data->map);
  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTile(data->window, newEntity->row, newEntity->col);
}

void mapEditorDataRemoveEntity(MapEditorData *data, UWORD entityNum) {
  mapRemoveEntity(data->map, entityNum);
  mapEditorDataSetSaved(data, FALSE);
}

UWORD mapEditorDataGetEntityCount(MapEditorData *data) {
  return data->map->entityCnt;
}

const Entity *mapEditorDataGetEntity(MapEditorData *data, UWORD entityNum) {
  return &data->map->entities[entityNum];
}

void mapEditorDataSetEntityNum(MapEditorData *data, UWORD entity, UBYTE entityNum) {
  Entity *entityObj = &data->map->entities[entity];
  entityObj->entityNum = entityNum;
  mapEditorDataSetSaved(data, FALSE);
}

UBYTE mapEditorDataGetEntityRow(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->row;
}

void mapEditorDataSetEntityRow(MapEditorData *data, UWORD entityNum, UBYTE row) {
  Entity *entity = &data->map->entities[entityNum];
  UBYTE oldRow = entity->row;
  UBYTE oldCol = entity->col;

  entity->row = row;

  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTile(data->window, oldRow, oldCol);
  mapEditorRefreshTile(data->window, row, oldCol);
}

UBYTE mapEditorDataGetEntityCol(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->col;
}

void mapEditorDataSetEntityCol(MapEditorData *data, UWORD entityNum, UBYTE col) {
  Entity *entity = &data->map->entities[entityNum];
  UBYTE oldRow = entity->row;
  UBYTE oldCol = entity->col;

  entity->col = col;

  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTile(data->window, oldRow, oldCol);
  mapEditorRefreshTile(data->window, oldRow, col);
}

UBYTE mapEditorDataGetEntityVRAMSlot(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->vramSlot;
}

void mapEditorDataSetEntityVRAMSlot(MapEditorData *data, UWORD entityNum, UBYTE vramSlot) {
  Entity *entity = &data->map->entities[entityNum];
  entity->vramSlot = vramSlot;
  mapEditorDataSetSaved(data, FALSE);
}

int mapEditorDataEntityGetTagCount(MapEditorData *data, UWORD entityNum) {
  return data->map->entities[entityNum].tagCnt;
}

void mapEditorDataEntityAddNewTag(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  entityAddNewTag(entity);
  mapEditorDataSetSaved(data, FALSE);
}

void mapEditorDataEntityDeleteTag(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  entityDeleteTag(entity, tagNum);
  mapEditorDataSetSaved(data, FALSE);
}

const char *mapEditorDataEntityGetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum) {
  return data->map->entities[entityNum].tags[tagNum].alias;
}

void mapEditorDataEntitySetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum, const char *newTagAlias) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  strcpy(tag->alias, newTagAlias);
  mapEditorDataSetSaved(data, FALSE);
}

UBYTE mapEditorDataEntityGetTagId(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->id;
}

void mapEditorDataEntitySetTagId(MapEditorData *data, UWORD entityNum, int tagNum, UBYTE newTagId) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  tag->id = newTagId;
  mapEditorDataSetSaved(data, FALSE);
}

UBYTE mapEditorDataEntityGetTagValue(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->value;
}

void mapEditorDataEntitySetTagValue(MapEditorData *data, UWORD entityNum, int tagNum, UBYTE newTagValue) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  tag->value = newTagValue;
  mapEditorDataSetSaved(data, FALSE);
}

BOOL mapEditorDataHasSong(MapEditorData *data) {
  return (BOOL)(data->map->songNum != 0);
}

void mapEditorDataClearSong(MapEditorData *data) {
  data->map->songNum = 0;
  mapEditorDataSetSaved(data, FALSE);
  mapEditorRefreshSong(data->window);
}

void mapEditorDataSetSong(MapEditorData *data, UWORD songNum) {
  data->map->songNum = songNum + 1;
  mapEditorDataSetSaved(data, FALSE);
  mapEditorRefreshSong(data->window);
}

UWORD mapEditorDataGetSong(MapEditorData *data) {
  return (UWORD)(data->map->songNum - 1);
}

void mapEditorDataClearTileset(MapEditorData *data) {
  data->map->tilesetNum = 0;

  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTilesetName(data->window);
  mapEditorRefreshTileDisplays(data->window);
}

/* TODO: move me elsewhere */
static void copyScaledTileset(UWORD *src, UWORD *dst) {
  struct BitMap srcBitMap;
  struct BitMap dstBitMap;
  struct BitScaleArgs scaleArgs;
  int tileNum;

  srcBitMap.BytesPerRow = 2;
  srcBitMap.Rows = 16;
  srcBitMap.Flags = 0;
  srcBitMap.Depth = 2;

  dstBitMap.BytesPerRow = 4;
  dstBitMap.Rows = 32;
  dstBitMap.Flags = 0;
  dstBitMap.Depth = 2;

  scaleArgs.bsa_SrcX = 0;
  scaleArgs.bsa_SrcY = 0;
  scaleArgs.bsa_SrcWidth = 16;
  scaleArgs.bsa_SrcHeight = 16;
  scaleArgs.bsa_XSrcFactor = 1;
  scaleArgs.bsa_YSrcFactor = 1;
  scaleArgs.bsa_DestX = 0;
  scaleArgs.bsa_DestY = 0;
  scaleArgs.bsa_XDestFactor = 2;
  scaleArgs.bsa_YDestFactor = 2;
  scaleArgs.bsa_SrcBitMap = &srcBitMap;
  scaleArgs.bsa_DestBitMap = &dstBitMap;
  scaleArgs.bsa_Flags = 0;

  for(tileNum = 0; tileNum < TILES_PER_SET; tileNum++) {
    srcBitMap.Planes[0] = (PLANEPTR)src;
    srcBitMap.Planes[1] = (PLANEPTR)(src + 16);

    dstBitMap.Planes[0] = (PLANEPTR)dst;
    dstBitMap.Planes[1] = (PLANEPTR)(dst + 64);

    BitMapScale(&scaleArgs);

    src += 32;
    dst += 128;
  }
}

BOOL mapEditorDataHasTileset(MapEditorData *data) {
  return (BOOL)(data->map->tilesetNum > 0);
}

UWORD mapEditorDataGetTileset(MapEditorData *data) {
  return (UWORD)(data->map->tilesetNum - 1);
}

void mapEditorDataSetTileset(MapEditorData *data, UWORD tilesetNum) {
  ProjectWindowData *projectData = data->window->parent->data;

  data->map->tilesetNum = tilesetNum + 1;

  copyScaledTileset(
    (UWORD*)projectDataGetTilesetImgs(projectData, tilesetNum),
    data->imageData);

  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTilesetName(data->window);
  mapEditorRefreshTileDisplays(data->window);
}

void mapEditorDataSetTileTo(MapEditorData *data, UBYTE row, UBYTE col, UBYTE to) {
  UBYTE tile = row * 10 + col;
  data->map->tiles[tile] = to;
  data->mapImages[tile].ImageData = mapEditorDataGetImageDataForTile(data, tile);
  mapEditorDataSetSaved(data, FALSE);

  mapEditorRefreshTile(data->window, row, col);
}

struct Image *mapEditorDataGetMapImage(MapEditorData *data, UBYTE tile) {
  return &data->mapImages[tile];
}

struct Image *mapEditorDataGetPaletteImage(MapEditorData *data, unsigned int tile) {
  return &data->paletteImages[tile];
}

BOOL mapEditorDataHasSelected(MapEditorData *data) {
  return (BOOL)(data->selected >= 0);
}

unsigned int mapEditorDataGetSelected(MapEditorData *data) {
  return (unsigned int)data->selected;
}

void mapEditorDataSetSelected(MapEditorData *data, unsigned int selected) {
  unsigned int oldSelected = data->selected;
  data->selected = selected;

  if(oldSelected >= 0) {
    mapEditorRefreshSelectedFrom(data->window, oldSelected);
  } else {
    mapEditorRefreshSelected(data->window);
  }
}
