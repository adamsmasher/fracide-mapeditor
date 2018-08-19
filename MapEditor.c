#include "MapEditor.h"

#include <exec/exec.h>
#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <graphics/scale.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/screen.h"

#include "easystructs.h"
#include "EntityBrowser.h"
#include "map.h"
#include "MapRequester.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"
#include "SaveStatus.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

#define MAP_EDITOR_WIDTH  536
#define MAP_EDITOR_HEIGHT 384

struct MapEditorData_tag {
  Map *map;
  UWORD mapNum;

  struct Gadget *tilesetNameGadget;
  struct Gadget *mapNameGadget;
  struct Gadget *songNameGadget;
  struct Gadget *leftGadget;
  struct Gadget *rightGadget;
  struct Gadget *upGadget;
  struct Gadget *downGadget;

  SaveStatus saveStatus;

  FrameworkWindow *tilesetRequester;
  SongRequester   *songRequester;
  FrameworkWindow *entityBrowser;

  struct Image paletteImages[TILESET_PALETTE_TILES_ACROSS * TILESET_PALETTE_TILES_HIGH];
  struct Image mapImages[MAP_TILES_ACROSS * MAP_TILES_HIGH];
  UWORD *imageData;

  int selected;

  char title[16];
};

static void newMapMenuItemClicked(FrameworkWindow*);
static void openMapMenuItemClicked(FrameworkWindow*);
static BOOL saveMap(FrameworkWindow*);
static BOOL saveMapAs(FrameworkWindow*);
static void revertMap(FrameworkWindow*);

static MenuSectionSpec newSection =
  { { "New", "N", MENU_ITEM_ENABLED, newMapMenuItemClicked },
    END_SECTION };

static MenuSectionSpec openSection =
  { { "Open", "O", MENU_ITEM_ENABLED, openMapMenuItemClicked },
    END_SECTION };

static MenuSectionSpec saveSection =
  { { "Save",       "S",         MENU_ITEM_ENABLED,  (Handler)saveMap   },
    { "Save As...", "A",         MENU_ITEM_ENABLED,  (Handler)saveMapAs },
    { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED,          revertMap },
  END_SECTION };

static MenuSectionSpec closeSection =
  { { "Close", "Q", MENU_ITEM_ENABLED, (Handler)tryToCloseWindow },
    END_SECTION };

static MenuSectionSpec *mapMenuSpec[] = {
  &newSection,
  &openSection,
  &saveSection,
  &closeSection,
  END_MENU
};

#define REVERT_MAP_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static MenuSpec mapEditorMenuSpec[] = {
  { "Map", &mapMenuSpec },
  END_MENUS
};

static void newMapMenuItemClicked(FrameworkWindow *mapEditor) {
  FrameworkWindow *projectWindow = mapEditor->parent;
  newMap(projectWindow);
}

static void openMapMenuItemClicked(FrameworkWindow *mapEditor) {
  FrameworkWindow *projectWindow = mapEditor->parent;
  openMap(projectWindow);
}

static void enableMapRevert(FrameworkWindow *mapEditor) {
  OnMenu(mapEditor->intuitionWindow, REVERT_MAP_MENU_ITEM);
}

static void disableMapRevert(FrameworkWindow *mapEditor) {
  OffMenu(mapEditor->intuitionWindow, REVERT_MAP_MENU_ITEM);
}

static void updateMapEditorTitle(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  char unsaved = data->saveStatus == SAVED ? '\0' : '*';
  if(data->mapNum) {
    sprintf(data->title, "Map %d%c", data->mapNum - 1, unsaved);
  } else {
    sprintf(data->title, "Map Editor%c", unsaved);
  }
  SetWindowTitles(mapEditor->intuitionWindow, data->title, (STRPTR)-1);
}

static void mapEditorSetSaveStatus(FrameworkWindow *mapEditor, SaveStatus saveStatus) {
  MapEditorData *data = mapEditor->data;
  if(saveStatus != data->saveStatus) {
    data->saveStatus = saveStatus;
    if(saveStatus == SAVED) {
      disableMapRevert(mapEditor);
    } else {
      enableMapRevert(mapEditor);
    }
    updateMapEditorTitle(mapEditor);
  }
}

static int saveMapRequester(FrameworkWindow *mapEditor) {
  char title[96];
  MapEditorData *data = mapEditor->data;

  sprintf(title, "Save Map %s", data->map->name);
  return spawnMapRequester(mapEditor, title);
}

static BOOL saveMapAs(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  int selected = saveMapRequester(mapEditor);
  if(!selected) {
    return FALSE;
  }

  if(!projectDataHasMap(projectData, selected - 1)) {
    if(projectDataSaveNewMap(projectData, data->map, selected - 1)) {
      fprintf(stderr, "saveMapAs: failed to save map\n");
      return FALSE;
    }
  } else {
    int response = EasyRequest(
      mapEditor->intuitionWindow,
      &saveIntoFullSlotEasyStruct,
      NULL,
      selected - 1,
      projectDataGetMapName(projectData, selected - 1));
    if(response) {
      projectDataOverwriteMap(projectData, data->map, selected - 1);
    } else {
      return FALSE;
    }
  }

  mapEditorSetMapNum(mapEditor, selected - 1);

  mapEditorSetSaveStatus(mapEditor, SAVED);

  projectDataUpdateMapName(projectData, selected - 1, data->map);

  return TRUE;
}

static BOOL saveMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  if(!data->mapNum) {
    return saveMapAs(mapEditor);
  } else {
    projectDataOverwriteMap(projectData, data->map, data->mapNum - 1);
    projectDataUpdateMapName(projectData, data->mapNum - 1, data->map);
    mapEditorSetSaveStatus(mapEditor, SAVED);
    return TRUE;
  }
}

static int confirmRevertMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  return EasyRequest(
    mapEditor->intuitionWindow,
    &confirmRevertMapEasyStruct,
    NULL,
    data->mapNum - 1,
    data->map->name);
}

static void revertMap(FrameworkWindow *mapEditor) {
  if(confirmRevertMap(mapEditor)) {
    FrameworkWindow *projectWindow = mapEditor->parent;
    MapEditorData *data = mapEditor->data;
    mapEditor->closed = 1;
    openMapNum(projectWindow, data->mapNum - 1);
  }
}

static BOOL unsavedMapEditorAlert(FrameworkWindow *mapEditor) {
  int response;
  MapEditorData *data = mapEditor->data;

  if(data->mapNum) {
    response = EasyRequest(
      mapEditor->intuitionWindow,
      &unsavedMapAlertEasyStructWithNum,
      NULL,
      data->mapNum - 1, data->map->name);
  } else {
    response = EasyRequest(
      mapEditor->intuitionWindow,
      &unsavedMapAlertEasyStructNoNum,
      NULL,
      data->map->name);
  }

  switch(response) {
    case 0: return FALSE;              /* cancel */
    case 1: return saveMap(mapEditor); /* save */
    case 2: return TRUE;               /* don't save */
    default:
      fprintf(stderr, "unsavedMapEditorAlert: unknown response %d\n", response);
      return FALSE;
    }
}

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  return (BOOL)(data->saveStatus == SAVED || unsavedMapEditorAlert(mapEditor));
}

#define TILE_WIDTH  16
#define TILE_HEIGHT 16

/* TODO: adjust based on screen */
#define MAP_BORDER_LEFT   28
#define MAP_BORDER_TOP    51
#define MAP_BORDER_WIDTH  (MAP_TILES_ACROSS * TILE_WIDTH  * 2 + 2)
#define MAP_BORDER_HEIGHT (MAP_TILES_HIGH   * TILE_HEIGHT * 2 + 2)

/* TODO: generate these dynamically */
static WORD mapBorderPoints[] = {
  0,                  0,
  MAP_BORDER_WIDTH-1, 0,
  MAP_BORDER_WIDTH-1, MAP_BORDER_HEIGHT-1,
  0,                  MAP_BORDER_HEIGHT-1,
  0,                  0
};
static struct Border mapBorder = {
  -1, -1,
  1, 1,
  JAM1,
  5, mapBorderPoints,
  NULL
};

/* TODO: adjust based on titlebar height */
#define CURRENT_TILESET_LEFT   378
#define CURRENT_TILESET_TOP    36
#define CURRENT_TILESET_WIDTH  144
#define CURRENT_TILESET_HEIGHT 12

#define CHOOSE_TILESET_LEFT    CURRENT_TILESET_LEFT
#define CHOOSE_TILESET_TOP     CURRENT_TILESET_TOP + CURRENT_TILESET_HEIGHT
#define CHOOSE_TILESET_HEIGHT  12
#define CHOOSE_TILESET_WIDTH   CURRENT_TILESET_WIDTH

#define TILESET_SCROLL_HEIGHT  TILE_HEIGHT * TILESET_PALETTE_TILES_HIGH * 2 + 2
#define TILESET_SCROLL_WIDTH   CHOOSE_TILESET_WIDTH - (TILE_WIDTH * TILESET_PALETTE_TILES_ACROSS * 2)
#define TILESET_SCROLL_LEFT    CURRENT_TILESET_LEFT + TILE_WIDTH * TILESET_PALETTE_TILES_ACROSS * 2 + 1
#define TILESET_SCROLL_TOP     CHOOSE_TILESET_TOP + CHOOSE_TILESET_HEIGHT + 8

#define TILESET_BORDER_LEFT   CURRENT_TILESET_LEFT
#define TILESET_BORDER_TOP    (TILESET_SCROLL_TOP + 1)
#define TILESET_BORDER_WIDTH  (TILE_WIDTH * TILESET_PALETTE_TILES_ACROSS * 2 + 2)
#define TILESET_BORDER_HEIGHT TILESET_SCROLL_HEIGHT

static WORD tilesetBorderPoints[] = {
  0,                      0,
  TILESET_BORDER_WIDTH-1, 0,
  TILESET_BORDER_WIDTH-1, TILESET_BORDER_HEIGHT-1,
  0,                      TILESET_BORDER_HEIGHT-1,
  0,                      0
};
static struct Border tilesetBorder = {
  -1, -1,
  1, 1,
  JAM1,
  5, tilesetBorderPoints,
  NULL
};

static void drawBorders(struct RastPort *rport) {
  DrawBorder(rport, &mapBorder, MAP_BORDER_LEFT, MAP_BORDER_TOP);
  DrawBorder(rport, &tilesetBorder,
    TILESET_BORDER_LEFT, TILESET_BORDER_TOP);
}

static void refreshMapEditor(FrameworkWindow *mapEditor) {
  drawBorders(mapEditor->intuitionWindow->RPort);
}

static void handleChooseTilesetClicked(FrameworkWindow *mapEditor) {
  FrameworkWindow *tilesetRequester;
  char title[32];
  MapEditorData *data = mapEditor->data;
  FrameworkWindow *projectWindow = mapEditor->parent;
  ProjectWindowData *projectData = projectWindow->data;

  if(data->tilesetRequester) {
    WindowToFront(data->tilesetRequester->intuitionWindow);
    goto done;
  }

  if(!projectDataHasTilesetPackage(projectData)) {
    int choice = EasyRequest(
      mapEditor->intuitionWindow,
      &noTilesetPackageLoadedEasyStruct,
      NULL);

    if(choice) {
      selectTilesetPackage(projectWindow);
    }
  }

  /* even after giving the user the opportunity to set the tileset
     package, we need to be sure they did so... */
  if(!projectDataHasTilesetPackage(projectData)) {
    goto done;
  }

  if(data->mapNum) {
    sprintf(title, "Choose Tileset For Map %d", data->mapNum - 1);
  } else {
    strcpy(title, "Choose Tileset");
  }

  tilesetRequester = newTilesetRequester(title, mapEditor);
  if(!tilesetRequester) {
    fprintf(stderr, "handleChooseTilesetClicked: couldn't make requester\n");
    goto error;
  }

  data->tilesetRequester = tilesetRequester;
done:
  return;
error:
  return;
}

static void updateMapEditorMapName(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  struct StringInfo *stringInfo = data->mapNameGadget->SpecialInfo;

  strcpy(data->map->name, stringInfo->Buffer);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void attachSongRequesterToMapEditor
(MapEditorData *data, SongRequester *songRequester) {
  data->songRequester = songRequester;
}

static void handleChangeSongClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(!data->songRequester) {
    char title[32];
    SongRequester *songRequester;

    if(data->mapNum) {
      sprintf(title, "Change Soundtrack For Map %d", data->mapNum - 1);
    } else {
      strcpy(title, "Change Soundtrack");
    }

    songRequester = newSongRequester(title);
    if(songRequester) {
      attachSongRequesterToMapEditor(data, songRequester);
      /* TODO: fix me */
      /* addWindowToSet(songRequester->window); */
    }
  } else {
    WindowToFront(data->songRequester->window->intuitionWindow);
  }
}

static void mapEditorClearSongUpdateUI(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  GT_SetGadgetAttrs(data->songNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, "N/A",
    TAG_END);
}

static void mapEditorClearSong(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  data->map->songNum = 0;
  mapEditorClearSongUpdateUI(mapEditor);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void moveToMap(FrameworkWindow *mapEditor, int mapNum) {
  FrameworkWindow *projectWindow = mapEditor->parent;

  if(ensureMapEditorSaved(mapEditor)) {
    if(openMapNum(projectWindow, mapNum - 1)) {
      mapEditor->closed = 1;
    }
  }
}

static void handleMapUp(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum - 16);
}

static void handleMapDown(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum + 16);
}

static void handleMapLeft(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum - 1);
}

static void handleMapRight(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum + 1);
}

static void openNewEntityBrowser(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  data->entityBrowser = newEntityBrowser(mapEditor, data->map, data->mapNum);
}

static void handleEntitiesClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(data->entityBrowser) {
    WindowToFront(data->entityBrowser->intuitionWindow);
  } else {
    openNewEntityBrowser(mapEditor);
  }
}

static int mapEditorClickInPalette(WORD x, WORD y) {
  return ((x > TILESET_BORDER_LEFT                        ) &&
          (x < TILESET_BORDER_LEFT + TILESET_BORDER_WIDTH ) &&
          (y > TILESET_BORDER_TOP                         ) &&
          (y < TILESET_BORDER_TOP  + TILESET_BORDER_HEIGHT));
}

static int mapEditorClickInMap(WORD x, WORD y) {
  return ((x > MAP_BORDER_LEFT                    ) &&
          (x < MAP_BORDER_LEFT + MAP_BORDER_WIDTH ) &&
          (y > MAP_BORDER_TOP                     ) &&
          (y < MAP_BORDER_TOP  + MAP_BORDER_HEIGHT));
}

static unsigned int mapEditorGetPaletteTileClicked(WORD x, WORD y) {
  unsigned int row = y;
  unsigned int col = x;

  row -= TILESET_BORDER_TOP;
  col -= TILESET_BORDER_LEFT;

  row >>= 5;
  col >>= 5;

  return (row << 2) + col;
}

static UBYTE mapEditorGetRowClicked(WORD y) {
  unsigned int row = y;
  row -= MAP_BORDER_TOP;
  row >>= 5;
  return (UBYTE)row;
}

static UBYTE mapEditorGetColClicked(WORD x) {
  unsigned int col = x;
  col -= MAP_BORDER_LEFT;
  col >>= 5;
  return (UBYTE)col;
}

static void redrawPaletteTile(FrameworkWindow *mapEditor, unsigned int tile) {
  MapEditorData *data = mapEditor->data;
  struct Image *image = &data->paletteImages[tile];
  struct Image *next = image->NextImage;
  image->NextImage = NULL;
  DrawImage(mapEditor->intuitionWindow->RPort, image,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);
  image->NextImage = next;
}

static WORD tileBorderPoints[] = {
  0,  0,
  31, 0,
  31, 31,
  0,  31,
  0,  0
};
static struct Border tileBorder = {
  0, 0,
  0, 0,
  COMPLEMENT,
  5, tileBorderPoints,
  NULL
};

static void mapEditorSetSelected(FrameworkWindow *mapEditor, unsigned int selected) {
  long row;
  long col;
  MapEditorData *data = mapEditor->data;

  if(data->selected >= 0) {
    redrawPaletteTile(mapEditor, data->selected);
  }

  data->selected = (int)selected;

  row = selected >> 2;
  col = selected & 0x03;

  DrawBorder(mapEditor->intuitionWindow->RPort, &tileBorder,
    TILESET_BORDER_LEFT + (col * 32),
    TILESET_BORDER_TOP  + (row * 32));
}

static void handleMapEditorPaletteClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  int tile = mapEditorGetPaletteTileClicked(x, y);
  mapEditorSetSelected(mapEditor, tile);
}

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

static void drawEntity(struct RastPort *rport, Entity *entity, int entityNum) {
  char             text[2];
  struct IntuiText itext;

  text[0] = '0' + entityNum;
  text[1] = '\0';

  itext.DrawMode  = COMPLEMENT;
  itext.ITextFont = NULL;
  itext.NextText  = NULL;
  itext.IText     = text;
  itext.LeftEdge  = entity->col * 32;
  itext.TopEdge   = entity->row * 32;

  PrintIText(rport, &itext, MAP_BORDER_LEFT + 1, MAP_BORDER_TOP + 1);

}

/* IN A DREAM WORLD: store the IntuiTexts in the map editor and render them all at once */
static void drawEntities(FrameworkWindow *mapEditor) {
  int i;
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[0];
  struct RastPort *rport = mapEditor->intuitionWindow->RPort;

  for(i = 0; i < data->map->entityCnt; i++) {
    drawEntity(rport, entity, i);
    entity++;
  }
}

static void mapEditorSetTilesetUpdateUI(FrameworkWindow *mapEditor, UWORD tilesetNumber) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  GT_SetGadgetAttrs(data->tilesetNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, projectDataGetTilesetName(projectData, tilesetNumber),
    TAG_END);

  copyScaledTileset(
    (UWORD*)projectDataGetTilesetImgs(projectData, tilesetNumber),
    data->imageData);

  DrawImage(mapEditor->intuitionWindow->RPort, data->paletteImages,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);

  DrawImage(mapEditor->intuitionWindow->RPort, data->mapImages,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);

  drawEntities(mapEditor);
}

static void *mapEditorDataGetImageDataForTile(MapEditorData *data, UBYTE tile) {
  return data->imageData + (tile << 7);
}

static void mapEditorSetTileTo(FrameworkWindow *mapEditor, UBYTE row, UBYTE col, UBYTE to) {
  MapEditorData *data = mapEditor->data;
  UBYTE tile = row * 10 + col;
  data->map->tiles[tile] = to;
  data->mapImages[tile].ImageData = mapEditorDataGetImageDataForTile(data, tile);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void redrawMapTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  UBYTE tile = row * 10 + col;
  struct Image *image = &data->mapImages[tile];
  struct Image *next = image->NextImage;
  int entity_i;

  image->NextImage = NULL;
  DrawImage(mapEditor->intuitionWindow->RPort, image,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);
  image->NextImage = next;

  if(entity_i = mapFindEntity(data->map, row, col)) {
    entity_i--;
    drawEntity(mapEditor->intuitionWindow->RPort, &data->map->entities[entity_i], entity_i);
  }
}

static void mapEditorSetTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  mapEditorSetTileTo(mapEditor, row, col, data->selected);
  redrawMapTile(mapEditor, row, col);
}

static void handleMapEditorMapClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  UBYTE col = mapEditorGetColClicked(x);
  UBYTE row = mapEditorGetRowClicked(y);
  mapEditorSetTile(mapEditor, row, col);
}

/* TODO: maybe we can use buttons for this... */
static void handleMapEditorClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  MapEditorData *data = mapEditor->data;

  if(data->map->tilesetNum) {
    if(mapEditorClickInPalette(x, y)) {
      handleMapEditorPaletteClick(mapEditor, x, y);
    } else if(mapEditorClickInMap(x, y)) {
      handleMapEditorMapClick(mapEditor, x, y);
    }
  }
}

static WindowKind mapEditorKind = {
  {
    40, 40, MAP_EDITOR_WIDTH, MAP_EDITOR_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|MOUSEBUTTONS|MENUPICK,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|ACTIVATE,
    NULL,
    NULL,
    "Map Editor",
    NULL,
    NULL,
    MAP_EDITOR_WIDTH,MAP_EDITOR_HEIGHT,
    MAP_EDITOR_WIDTH,MAP_EDITOR_HEIGHT,
    CUSTOMSCREEN
  },
  (MenuSpec*)        mapEditorMenuSpec,
  (RefreshFunction)  refreshMapEditor,
  (CanCloseFunction) ensureMapEditorSaved,
  (CloseFunction)    NULL,
  (ClickFunction)    handleMapEditorClick
};

BOOL isMapEditor(FrameworkWindow *window) {
  return (BOOL)(window->kind == &mapEditorKind);
}

BOOL mapEditorHasSongRequester(MapEditorData *data) {
  return (BOOL)(data->songRequester != NULL);
}

BOOL mapEditorHasEntityBrowser(MapEditorData *data) {
  return (BOOL)(data->entityBrowser != NULL);
}

#define MAP_NAME_LEFT   (MAP_BORDER_LEFT  + 80)
#define MAP_NAME_TOP    18
#define MAP_NAME_WIDTH  (MAP_BORDER_WIDTH - 81)
#define MAP_NAME_HEIGHT 14

#define ENTITIES_LEFT   (TILESET_BORDER_LEFT - 1)
#define ENTITIES_TOP    (TILESET_BORDER_TOP + TILESET_BORDER_HEIGHT + 10)
#define ENTITIES_WIDTH  (TILESET_BORDER_WIDTH + TILESET_SCROLL_WIDTH)
#define ENTITIES_HEIGHT 14

#define SONG_NAME_LEFT   (MAP_BORDER_LEFT + 95)
#define SONG_NAME_TOP    (MAP_BORDER_TOP + MAP_BORDER_HEIGHT + 20)
#define SONG_NAME_WIDTH  (MAP_BORDER_WIDTH - 182)
#define SONG_NAME_HEIGHT 14

#define SONG_CHANGE_LEFT   (SONG_NAME_LEFT + SONG_NAME_WIDTH - 2)
#define SONG_CHANGE_TOP    SONG_NAME_TOP
#define SONG_CHANGE_WIDTH  76
#define SONG_CHANGE_HEIGHT 14

#define SONG_CLEAR_LEFT   (SONG_CHANGE_LEFT + SONG_CHANGE_WIDTH - 2)
#define SONG_CLEAR_TOP    SONG_CHANGE_TOP
#define SONG_CLEAR_WIDTH  14
#define SONG_CLEAR_HEIGHT 14

#define MAP_UP_LEFT   (MAP_BORDER_LEFT - 3)
#define MAP_UP_TOP    (MAP_BORDER_TOP - 15)
#define MAP_UP_WIDTH  (MAP_BORDER_WIDTH + 4)
#define MAP_UP_HEIGHT 14

#define MAP_DOWN_LEFT   MAP_UP_LEFT
#define MAP_DOWN_TOP    (MAP_BORDER_TOP + MAP_BORDER_HEIGHT - 1)
#define MAP_DOWN_WIDTH  MAP_UP_WIDTH
#define MAP_DOWN_HEIGHT MAP_UP_HEIGHT

#define MAP_LEFT_LEFT   (MAP_BORDER_LEFT - 15)
#define MAP_LEFT_TOP    MAP_BORDER_TOP - 2
#define MAP_LEFT_WIDTH  14
#define MAP_LEFT_HEIGHT MAP_BORDER_HEIGHT + 2

#define MAP_RIGHT_LEFT   (MAP_BORDER_LEFT + MAP_BORDER_WIDTH - 1)
#define MAP_RIGHT_TOP    MAP_LEFT_TOP
#define MAP_RIGHT_WIDTH  MAP_LEFT_WIDTH
#define MAP_RIGHT_HEIGHT MAP_LEFT_HEIGHT

#define IMAGE_DATA_SIZE (TILES_PER_SET * 256)

static StringSpec mapNameSpec = {
  MAP_NAME_LEFT,  MAP_NAME_TOP,
  MAP_NAME_WIDTH, MAP_NAME_HEIGHT,
  "Map Name:",
  TEXT_ON_LEFT,
  ENABLED,
  updateMapEditorMapName
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
  handleChooseTilesetClicked
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
  handleChangeSongClicked
};

static ButtonSpec songClearSpec = {
  SONG_CLEAR_LEFT,  SONG_CLEAR_TOP,
  SONG_CLEAR_WIDTH, SONG_CLEAR_HEIGHT,
  "X",
  TEXT_INSIDE,
  ENABLED,
  mapEditorClearSong
};

static ButtonSpec mapLeftSpec = {
  MAP_LEFT_LEFT,  MAP_LEFT_TOP,
  MAP_LEFT_WIDTH, MAP_LEFT_HEIGHT,
  "<",
  TEXT_INSIDE,
  DISABLED,
  handleMapLeft
};

static ButtonSpec mapRightSpec = {
  MAP_RIGHT_LEFT,  MAP_RIGHT_TOP,
  MAP_RIGHT_WIDTH, MAP_RIGHT_HEIGHT,
  ">",
  TEXT_INSIDE,
  DISABLED,
  handleMapRight
};

static ButtonSpec mapUpSpec = {
  MAP_UP_LEFT,  MAP_UP_TOP,
  MAP_UP_WIDTH, MAP_UP_HEIGHT,
  "^",
  TEXT_INSIDE,
  DISABLED,
  handleMapUp
};

static ButtonSpec mapDownSpec = {
  MAP_DOWN_LEFT,  MAP_DOWN_TOP,
  MAP_DOWN_WIDTH, MAP_DOWN_HEIGHT,
  "v",
  TEXT_INSIDE,
  DISABLED,
  handleMapDown
};

static ButtonSpec entitiesSpec = {
  ENTITIES_LEFT,  ENTITIES_TOP,
  ENTITIES_WIDTH, ENTITIES_HEIGHT,
  "Entities...",
  TEXT_INSIDE,
  ENABLED,
  handleEntitiesClicked
};

static struct EasyStruct tilesetOutOfBoundsEasyStruct = {
  sizeof(struct EasyStruct),
  0,
  "Tileset Not In New Tileset Package",
  "This map had tileset %ld, which does not exist\nin the new package.\nThe tileset has been removed from this map.",
  "OK"
};

static void initMapEditorPaletteImages(MapEditorData *data) {
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

static void initMapEditorMapImages(MapEditorData *data) {
  int top, left, row, col;
  struct Image *i = data->mapImages;
  UWORD *imageData = data->imageData;

  top = 0;
  for(row = 0; row < MAP_TILES_HIGH; row++) {
    left = 0;
    for(col = 0; col < MAP_TILES_ACROSS; col++) {
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

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  if(data->map->tilesetNum) {
    drawEntity(mapEditor->intuitionWindow->RPort, entity, entityNum);
  }
}

static void mapEditorClearTilesetUI(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  struct RastPort *rport = mapEditor->intuitionWindow->RPort;

  GT_SetGadgetAttrs(data->tilesetNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, "N/A",
    TAG_END);

  SetAPen(rport, 0);
  SetDrMd(rport, JAM1);

  RectFill(rport,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP,
    TILESET_BORDER_LEFT + TILESET_BORDER_WIDTH  - 3,
    TILESET_BORDER_TOP  + TILESET_BORDER_HEIGHT - 3);

  RectFill(rport,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP,
    MAP_BORDER_LEFT + MAP_BORDER_WIDTH  - 3,
    MAP_BORDER_TOP  + MAP_BORDER_HEIGHT - 3);
}

void mapEditorRefreshTileset(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  if(data->map->tilesetNum) {
    if(data->map->tilesetNum - 1 < projectDataGetTilesetCount(projectData)) {
      mapEditorSetTilesetUpdateUI(mapEditor, data->map->tilesetNum - 1);
    } else {
      mapEditorClearTilesetUI(mapEditor);
      EasyRequest(mapEditor->intuitionWindow, &tilesetOutOfBoundsEasyStruct, NULL,
        data->map->tilesetNum - 1);
      data->map->tilesetNum = 0;
      mapEditorSetSaveStatus(mapEditor, UNSAVED);
    }
  }
}

static void updateTilesetRequesterChildren(FrameworkWindow *mapEditor) {
  FrameworkWindow *i = mapEditor->children;
  while(i) {
    if(isTilesetRequesterWindow(i)) {
      refreshTilesetRequesterList(i);
    }
    i = i->next;
  }
}

void mapEditorUpdateTileDisplays(FrameworkWindow *mapEditor) {
  updateTilesetRequesterChildren(mapEditor);
  mapEditorRefreshTileset(mapEditor);
}

void mapEditorSetTileset(FrameworkWindow *mapEditor, UWORD tilesetNumber) {
  MapEditorData *data = mapEditor->data;
  data->map->tilesetNum = tilesetNumber + 1;
  mapEditorSetTilesetUpdateUI(mapEditor, tilesetNumber);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void mapEditorSetSongUpdateUI(FrameworkWindow *mapEditor, UWORD songNumber) {
  MapEditorData *data = mapEditor->data;
  GT_SetGadgetAttrs(data->songNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, projectDataGetSongName(mapEditor->parent->data, songNumber),
    TAG_END);
}

void mapEditorSetSong(FrameworkWindow *mapEditor, UWORD songNumber) {
  MapEditorData *data = mapEditor->data;
  data->map->songNum = songNumber + 1;
  mapEditorSetSongUpdateUI(mapEditor, songNumber);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

void mapEditorRefreshSong(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(data->map->songNum) {
     mapEditorSetSongUpdateUI(mapEditor, data->map->songNum - 1);
  }
}

void mapEditorRedrawTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  if(data->map->tilesetNum) {
    redrawMapTile(mapEditor, row, col);
  }
}

/* results are undefined if the map editor does not have a map */
UWORD mapEditorGetMapNum(MapEditorData *data) {
  return (UWORD)(data->mapNum - 1);
}

void mapEditorSetMapNum(FrameworkWindow *mapEditor, UWORD mapNum) {
  MapEditorData *data = mapEditor->data;  

  BOOL upDisabled = mapNum < 16 ? TRUE : FALSE;
  BOOL downDisabled = mapNum >= 112 ? TRUE : FALSE;
  BOOL leftDisabled = mapNum % 16 == 0 ? TRUE : FALSE;
  BOOL rightDisabled = mapNum % 16 == 15 ? TRUE : FALSE;

  data->mapNum = mapNum + 1;

  GT_SetGadgetAttrs(data->upGadget, mapEditor->intuitionWindow, NULL,
    GA_Disabled, upDisabled,
    TAG_END);

  GT_SetGadgetAttrs(data->downGadget, mapEditor->intuitionWindow, NULL,
    GA_Disabled, downDisabled,
    TAG_END);

  GT_SetGadgetAttrs(data->leftGadget, mapEditor->intuitionWindow, NULL,
    GA_Disabled, leftDisabled,
    TAG_END);

  GT_SetGadgetAttrs(data->rightGadget, mapEditor->intuitionWindow, NULL,
    GA_Disabled, rightDisabled,
    TAG_END);

  updateMapEditorTitle(mapEditor);
}

static FrameworkWindow *newMapEditor(FrameworkWindow *parent) {
  FrameworkWindow *mapEditor;
  struct Gadget *gadgets;

  MapEditorData *data = malloc(sizeof(MapEditorData));
  if(!data) {
    fprintf(stderr, "newMapEditor: failed to allocate MapEditorData\n");
    goto error;
  }

  data->imageData = AllocMem(IMAGE_DATA_SIZE, MEMF_CHIP);
  if(!data->imageData) {
    fprintf(stderr, "newMapEditor: failed to allocate image data\n");
    goto error_freeData;
  }
  initMapEditorPaletteImages(data);
  initMapEditorMapImages(data);

  gadgets = buildGadgets(
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

  if(!gadgets) {
    fprintf(stderr, "newMapEditor: failed to create gadgets\n");
    goto error_freeImageData;
  }

  mapEditor = openChildWindow(parent, &mapEditorKind, gadgets);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditor: failed to open window\n");
    goto error_freeImageData;
  }

  refreshMapEditor(mapEditor);

  data->tilesetRequester = NULL;
  data->songRequester    = NULL;
  data->entityBrowser    = NULL;
  data->selected         = -1;

  mapEditor->data = data;

  return mapEditor;

error_freeImageData:
  FreeMem(data->imageData, IMAGE_DATA_SIZE);
error_freeData:
  free(data);
error:
    return NULL;
}

FrameworkWindow *newMapEditorNewMap(FrameworkWindow *parent) {
  Map *map;
  MapEditorData *data;
  FrameworkWindow *mapEditor;

  map = allocMap();
  if(!map) {
    fprintf(stderr, "newMapEditorNewMap: failed to allocate map\n");
    goto error;
  }

  mapEditor = newMapEditor(parent);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditorNewMap: failed to create map editor\n");
    goto error_freeMap;
  }
  
  data = mapEditor->data;
  data->map = map;
  data->mapNum = 0;

  return mapEditor;

error_freeMap:
  free(map);
error:
  return NULL;
}

static void mapEditorDataInitializeImages(MapEditorData *data) {
  Map *map = data->map;
  int i;
  for(i = 0; i < MAP_TILES_HIGH * MAP_TILES_WIDE; i++) {
    data->mapImages[i].ImageData = mapEditorDataGetImageDataForTile(data, map->tiles[i]);
  }
}

FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map *map, int mapNum) {
  Map *mapCopy;
  FrameworkWindow *mapEditor;
  MapEditorData *data;

  mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "newMapEditorWithMap: failed to copy map\n");
    goto error;
  }

  mapEditor = newMapEditor(parent);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditorWithMap: failed to create map editor\n");
    goto error_freeMap;
  }

  data = mapEditor->data;

  GT_SetGadgetAttrs(data->mapNameGadget, mapEditor->intuitionWindow, NULL,
    GTST_String, map->name,
    TAG_END);

  data->map = mapCopy;

  if(data->map->tilesetNum) {
    mapEditorDataInitializeImages(data);
    mapEditorSetTilesetUpdateUI(mapEditor, map->tilesetNum - 1);
  }

  if(map->songNum) {
    mapEditorSetSong(mapEditor, map->songNum - 1);
  }

  mapEditorSetMapNum(mapEditor, mapNum);
  return mapEditor;

error_freeMap:
  free(mapCopy);
error:
  return NULL;
}

void mapEditorAddNewEntity(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  mapAddNewEntity(data->map);
  /* draw the new entity */
  mapEditorDrawEntity(mapEditor, mapEditorEntityCount(mapEditor->data) - 1);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

void mapEditorRemoveEntity(FrameworkWindow *mapEditor, UWORD entityNum) {
  MapEditorData *data = mapEditor->data;
  mapRemoveEntity(data->map, entityNum);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

UBYTE mapEditorGetEntityRow(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->row;
}

void mapEditorSetEntityRow(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE row) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  UBYTE oldRow = entity->row;
  UBYTE oldCol = entity->col;

  entity->row = row;

  mapEditorSetSaveStatus(mapEditor, UNSAVED);
  mapEditorDrawEntity(mapEditor, entityNum);
  mapEditorRedrawTile(mapEditor, oldRow, oldCol);
}

UBYTE mapEditorGetEntityCol(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->col;
}

void mapEditorSetEntityCol(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  UBYTE oldRow = entity->row;
  UBYTE oldCol = entity->col;

  entity->col = col;

  mapEditorSetSaveStatus(mapEditor, UNSAVED);
  mapEditorDrawEntity(mapEditor, entityNum);
  mapEditorRedrawTile(mapEditor, oldRow, oldCol);
}

UBYTE mapEditorGetEntityVRAMSlot(MapEditorData *data, UWORD entityNum) {
  Entity *entity = &data->map->entities[entityNum];
  return entity->vramSlot;
}

void mapEditorSetEntityVRAMSlot(FrameworkWindow *mapEditor, UWORD entityNum, UBYTE vramSlot) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  entity->vramSlot = vramSlot;
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

UWORD mapEditorEntityCount(MapEditorData *data) {
  return data->map->entityCnt;
}

void mapEditorEntityAddNewTag(FrameworkWindow *mapEditor, UWORD entityNum) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  entityAddNewTag(entity);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

void mapEditorEntityDeleteTag(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  entityDeleteTag(entity, tagNum);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

int mapEditorEntityGetTagCount(MapEditorData *data, UWORD entityNum) {
  return data->map->entities[entityNum].tagCnt;
}

void mapEditorEntitySetTagAlias(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, const char *newTagAlias) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  strcpy(tag->alias, newTagAlias);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

const char *mapEditorEntityGetTagAlias(MapEditorData *data, UWORD entityNum, int tagNum) {
  return data->map->entities[entityNum].tags[tagNum].alias;
}

UBYTE mapEditorEntityGetTagId(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->id;
}

void mapEditorEntitySetTagId(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagId) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  tag->id = newTagId;
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

UBYTE mapEditorEntityGetTagValue(MapEditorData *data, UWORD entityNum, int tagNum) {
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  return tag->value;
}

void mapEditorEntitySetTagValue(FrameworkWindow *mapEditor, UWORD entityNum, int tagNum, UBYTE newTagValue) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  Frac_tag *tag = &entity->tags[tagNum];
  tag->value = newTagValue;
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}
