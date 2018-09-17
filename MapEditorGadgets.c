#include "MapEditorGadgets.h"

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>

#include "framework/Gadgets.h"

#include "Map.h"
#include "MapEditor.h"
#include "MapEditorConstants.h"

static StringSpec mapNameSpec = {
  MAP_NAME_LEFT,  MAP_NAME_TOP,
  MAP_NAME_WIDTH, MAP_NAME_HEIGHT,
  "Map Name:",
  TEXT_ON_LEFT,
  ENABLED,
  mapEditorUpdateMapName
};

static TextSpec currentTilesetSpec = {
  CURRENT_TILESET_LEFT,  CURRENT_TILESET_TOP,
  CURRENT_TILESET_WIDTH, CURRENT_TILESET_HEIGHT,
  "Current Tileset",
  TEXT_ABOVE,
  "N/A",
  BORDERED
};

static ButtonSpec chooseTilesetSpec = {
  CHOOSE_TILESET_LEFT, CHOOSE_TILESET_TOP,
  CHOOSE_TILESET_WIDTH, CHOOSE_TILESET_HEIGHT,
  "Choose Tileset...",
  TEXT_INSIDE,
  ENABLED,
  mapEditorChooseTilesetClicked
};

static ScrollerSpec tilesetScrollSpec = {
  TILESET_SCROLL_LEFT,  TILESET_SCROLL_TOP,
  TILESET_SCROLL_WIDTH, TILESET_SCROLL_HEIGHT,
  DISABLED,
  VERTICAL
};

static TextSpec songNameSpec = {
  SONG_NAME_LEFT,  SONG_NAME_TOP,
  SONG_NAME_WIDTH, SONG_NAME_HEIGHT,
  "Soundtrack:",
  TEXT_ON_LEFT,
  "N/A",
  BORDERED
};

static ButtonSpec songChangeSpec = {
  SONG_CHANGE_LEFT,  SONG_CHANGE_TOP,
  SONG_CHANGE_WIDTH, SONG_CHANGE_HEIGHT,
  "Change...",
  TEXT_INSIDE,
  ENABLED,
  mapEditorChangeSongClicked
};

static ButtonSpec songClearSpec = {
  SONG_CLEAR_LEFT,  SONG_CLEAR_TOP,
  SONG_CLEAR_WIDTH, SONG_CLEAR_HEIGHT,
  "X",
  TEXT_INSIDE,
  ENABLED,
  mapEditorClearSongClicked
};

static ButtonSpec mapLeftSpec = {
  MAP_LEFT_LEFT,  MAP_LEFT_TOP,
  MAP_LEFT_WIDTH, MAP_LEFT_HEIGHT,
  "<",
  TEXT_INSIDE,
  DISABLED,
  mapEditorMapLeftClicked
};

static ButtonSpec mapRightSpec = {
  MAP_RIGHT_LEFT,  MAP_RIGHT_TOP,
  MAP_RIGHT_WIDTH, MAP_RIGHT_HEIGHT,
  ">",
  TEXT_INSIDE,
  DISABLED,
  mapEditorMapRightClicked
};

static ButtonSpec mapUpSpec = {
  MAP_UP_LEFT,  MAP_UP_TOP,
  MAP_UP_WIDTH, MAP_UP_HEIGHT,
  "^",
  TEXT_INSIDE,
  DISABLED,
  mapEditorMapUpClicked
};

static ButtonSpec mapDownSpec = {
  MAP_DOWN_LEFT,  MAP_DOWN_TOP,
  MAP_DOWN_WIDTH, MAP_DOWN_HEIGHT,
  "v",
  TEXT_INSIDE,
  DISABLED,
  mapEditorMapDownClicked
};

static ButtonSpec entitiesSpec = {
  ENTITIES_LEFT,  ENTITIES_TOP,
  ENTITIES_WIDTH, ENTITIES_HEIGHT,
  "Entities...",
  TEXT_INSIDE,
  ENABLED,
  mapEditorEntitiesClicked
};

WindowGadgets *newMapEditorGadgets(void) {
  MapEditorGadgets *data;
  WindowGadgets *gadgets;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "newMapEditorGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  data = malloc(sizeof(MapEditorGadgets));
  if(!data) {
    fprintf(stderr, "newMapEditorGadgets: couldn't allocate map editor gadgets\n");
    goto error_freeWindowGadgets;
  }

  gadgets->glist = buildGadgets(
    makeTextGadget(&currentTilesetSpec),    &data->tilesetNameGadget,
    makeButtonGadget(&chooseTilesetSpec),   NULL,
    makeScrollerGadget(&tilesetScrollSpec), NULL,
    makeStringGadget(&mapNameSpec),         &data->mapNameGadget,
    makeTextGadget(&songNameSpec),          &data->songNameGadget,
    makeButtonGadget(&songChangeSpec),      NULL,
    makeButtonGadget(&songClearSpec),       NULL,
    makeButtonGadget(&mapLeftSpec),         &data->leftGadget,
    makeButtonGadget(&mapRightSpec),        &data->rightGadget,
    makeButtonGadget(&mapUpSpec),           &data->upGadget,
    makeButtonGadget(&mapDownSpec),         &data->downGadget,
    makeButtonGadget(&entitiesSpec),        NULL,
    NULL);
  if(!gadgets->glist) {
    fprintf(stderr, "newMapEditorGadgets: couldn't build gadgets\n");
    goto error_freeMapEditorGadgets;
  }

  gadgets->data = data;

  return gadgets;

error_freeMapEditorGadgets:
  free(data);
error_freeWindowGadgets:
  free(gadgets);
error:
  return NULL;
}

void freeMapEditorGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}
