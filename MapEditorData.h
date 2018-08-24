#ifndef MAP_EDITOR_DATA_H
#define MAP_EDITOR_DATA_H

#include "MapEditorGadgets.h"
#include "SaveStatus.h"

struct MapEditorData_tag {
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
};

#endif

