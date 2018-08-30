#include "MapEditorData.h"

#include <exec/exec.h>
#include <proto/exec.h>

#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>
#include <string.h>

#include "MapEditor.h"
#include "ProjectWindowData.h"
#include "TilesetPackage.h"

#define IMAGE_DATA_SIZE (TILES_PER_SET * 256)

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

void mapEditorDataInitImages(MapEditorData *data) {
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

  data->tilesetRequester = NULL;
  data->songRequester    = NULL;
  data->entityBrowser    = NULL;
  data->selected         = -1;

  return data;
error_freeData:
  free(data);
error:
  return NULL;
}

void freeMapEditorData(MapEditorData* data) {
  /* TODO: free other things as well */
  FreeMem(data->imageData, IMAGE_DATA_SIZE);
}

static void mapEditorDataUpdateTitle(MapEditorData *data) {
  char unsaved = data->saveStatus == SAVED ? '\0' : '*';
  if(data->mapNum) {
    sprintf(data->title, "Map %d%c", data->mapNum - 1, unsaved);
  } else {
    sprintf(data->title, "Map Editor%c", unsaved);
  }
  mapEditorRefreshTitle(data->window);
}

void mapEditorDataSetSaveStatus(MapEditorData *data, SaveStatus saveStatus) {
  if(saveStatus != data->saveStatus) {
    data->saveStatus = saveStatus;
    mapEditorRefreshRevertMap(data->window);
    mapEditorDataUpdateTitle(data);
  }
}

/* results are undefined if the map editor does not have a map */
UWORD mapEditorDataGetMapNum(MapEditorData *data) {
  return (UWORD)(data->mapNum - 1);
}

void mapEditorDataSetMapNum(MapEditorData *data, UWORD mapNum) {
  MapEditorGadgets *gadgets = &data->gadgets;
  struct Window *window = data->window->intuitionWindow;

  BOOL upDisabled = mapNum < 16 ? TRUE : FALSE;
  BOOL downDisabled = mapNum >= 112 ? TRUE : FALSE;
  BOOL leftDisabled = mapNum % 16 == 0 ? TRUE : FALSE;
  BOOL rightDisabled = mapNum % 16 == 15 ? TRUE : FALSE;

  data->mapNum = mapNum + 1;

  GT_SetGadgetAttrs(gadgets->upGadget, window, NULL,
    GA_Disabled, upDisabled,
    TAG_END);

  GT_SetGadgetAttrs(gadgets->downGadget, window, NULL,
    GA_Disabled, downDisabled,
    TAG_END);

  GT_SetGadgetAttrs(gadgets->leftGadget, window, NULL,
    GA_Disabled, leftDisabled,
    TAG_END);

  GT_SetGadgetAttrs(gadgets->rightGadget, window, NULL,
    GA_Disabled, rightDisabled,
    TAG_END);

  mapEditorDataUpdateTitle(data);
}

void mapEditorDataSetMapName(MapEditorData *data, const char *mapName) {
  strcpy(data->map->name, mapName);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

void mapEditorDataSetSongRequester(MapEditorData *data, SongRequester *songRequester) {
  data->songRequester = songRequester;
}

void *mapEditorDataGetImageDataForTile(MapEditorData *data, UBYTE tile) {
  return data->imageData + (tile << 7);
}

BOOL mapEditorDataHasSongRequester(MapEditorData *data) {
  return (BOOL)(data->songRequester != NULL);
}

BOOL mapEditorDataHasEntityBrowser(MapEditorData *data) {
  return (BOOL)(data->entityBrowser != NULL);
}

void mapEditorDataAddNewEntity(MapEditorData *data) {
  mapAddNewEntity(data->map);
  mapEditorDrawEntity(data->window, data->map->entityCnt - 1);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

void mapEditorDataRemoveEntity(MapEditorData *data, UWORD entityNum) {
  mapRemoveEntity(data->map, entityNum);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

UWORD mapEditorDataGetEntityCount(MapEditorData *data) {
  return data->map->entityCnt;
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

  mapEditorDataSetSaveStatus(data, UNSAVED);
  mapEditorDrawEntity(data->window, entityNum);
  mapEditorRedrawTile(data->window, oldRow, oldCol);
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

  mapEditorDataSetSaveStatus(data, UNSAVED);
  mapEditorDrawEntity(data->window, entityNum);
  mapEditorRedrawTile(data->window, oldRow, oldCol);
}

UBYTE mapEditorDataGetEntityVRAMSlot(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->vramSlot;
}

void mapEditorDataSetEntityVRAMSlot(MapEditorData *data, UWORD entityNum, UBYTE vramSlot) {
  Entity *entity = &data->map->entities[entityNum];
  entity->vramSlot = vramSlot;
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

int mapEditorDataEntityGetTagCount(MapEditorData *data, UWORD entityNum) {
  return data->map->entities[entityNum].tagCnt;
}

void mapEditorDataEntityAddNewTag(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  entityAddNewTag(entity);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

void mapEditorDataEntityDeleteTag(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  entityDeleteTag(entity, tagNum);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

const char *mapEditorDataEntityGetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum) {
  return data->map->entities[entityNum].tags[tagNum].alias;
}

void mapEditorDataEntitySetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum, const char *newTagAlias) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  strcpy(tag->alias, newTagAlias);
  mapEditorDataSetSaveStatus(data, UNSAVED);
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
  mapEditorDataSetSaveStatus(data, UNSAVED);
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
  mapEditorDataSetSaveStatus(data, UNSAVED);
}

void mapEditorDataClearSong(MapEditorData *data) {
  MapEditorGadgets *gadgets = &data->gadgets;

  data->map->songNum = 0;

  mapEditorDataSetSaveStatus(data, UNSAVED);

  GT_SetGadgetAttrs(gadgets->songNameGadget, data->window->intuitionWindow, NULL,
    GTTX_Text, "N/A",
    TAG_END);
}

void mapEditorDataSetSong(MapEditorData *data, UWORD songNum) {
  MapEditorGadgets *gadgets = &data->gadgets;
  FrameworkWindow *mapEditor = data->window;
  ProjectWindowData *projectData = mapEditor->parent->data;

  data->map->songNum = songNum + 1;

  GT_SetGadgetAttrs(gadgets->songNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, projectDataGetSongName(projectData, songNum),
    TAG_END);

  mapEditorDataSetSaveStatus(data, UNSAVED);  
}

void mapEditorDataClearTileset(MapEditorData *data) {
  MapEditorGadgets *gadgets = &data->gadgets;

  data->map->tilesetNum = 0;

  GT_SetGadgetAttrs(gadgets->tilesetNameGadget, data->window->intuitionWindow, NULL,
    GTTX_Text, "N/A",
    TAG_END);

  mapEditorDataSetSaveStatus(data, UNSAVED);

  mapEditorRefreshTileDisplays(data->window);
}

void mapEditorDataSetTileset(MapEditorData *data, UWORD tilesetNum) {
  MapEditorGadgets *gadgets = &data->gadgets;
  FrameworkWindow *mapEditor = data->window;
  ProjectWindowData *projectData = mapEditor->parent->data;

  data->map->tilesetNum = tilesetNum + 1;

  copyScaledTileset(
    (UWORD*)projectDataGetTilesetImgs(projectData, tilesetNum),
    data->imageData);

  GT_SetGadgetAttrs(gadgets->tilesetNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, projectDataGetTilesetName(projectData, tilesetNum),
    TAG_END);

  mapEditorDataSetSaveStatus(data, UNSAVED);

  mapEditorRefreshTileDisplays(data->window);
}

void mapEditorDataSetTileTo(MapEditorData *data, UBYTE row, UBYTE col, UBYTE to) {
  UBYTE tile = row * 10 + col;
  data->map->tiles[tile] = to;
  data->mapImages[tile].ImageData = mapEditorDataGetImageDataForTile(data, tile);
  mapEditorDataSetSaveStatus(data, UNSAVED);
}
