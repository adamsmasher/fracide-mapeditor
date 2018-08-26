#include "MapEditorData.h"

void mapEditorDataInitPaletteImages(MapEditorData *data) {
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

void mapEditorDataInitMapImages(MapEditorData *data) {
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

void mapEditorDataUpdateTitle(MapEditorData *data) {
  char unsaved = data->saveStatus == SAVED ? '\0' : '*';
  if(data->mapNum) {
    sprintf(data->title, "Map %d%c", data->mapNum - 1, unsaved);
  } else {
    sprintf(data->title, "Map Editor%c", unsaved);
  }
}

/* results are undefined if the map editor does not have a map */
UWORD mapEditorDataGetMapNum(MapEditorData *data) {
  return (UWORD)(data->mapNum - 1);
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

UWORD mapEditorDataGetEntityCount(MapEditorData *data) {
  return data->map->entityCnt;
}

UBYTE mapEditorDataGetEntityRow(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->row;
}

UBYTE mapEditorDataGetEntityCol(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->col;
}

UBYTE mapEditorDataGetEntityVRAMSlot(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->vramSlot;
}

int mapEditorDataEntityGetTagCount(MapEditorData *data, UWORD entityNum) {
  return data->map->entities[entityNum].tagCnt;
}

const char *mapEditorDataEntityGetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum) {
  return data->map->entities[entityNum].tags[tagNum].alias;
}

UBYTE mapEditorDataEntityGetTagId(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->id;
}

UBYTE mapEditorDataEntityGetTagValue(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->value;
}
