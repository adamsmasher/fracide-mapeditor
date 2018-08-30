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
#include "MapEditorConstants.h"
#include "MapEditorData.h"
#include "MapEditorGadgets.h"
#include "MapEditorMenu.h"
#include "MapRequester.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"
#include "SaveStatus.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

#define MAP_EDITOR_WIDTH  536
#define MAP_EDITOR_HEIGHT 384

void mapEditorNewMap(FrameworkWindow *mapEditor) {
  FrameworkWindow *projectWindow = mapEditor->parent;
  projectWindowNewMap(projectWindow);
}

void mapEditorOpenMap(FrameworkWindow *mapEditor) {
  FrameworkWindow *projectWindow = mapEditor->parent;
  mapEditorOpenMap(projectWindow);
}

static void mapEditorEnableRevertMap(FrameworkWindow *mapEditor) {
  mapEditorMenuEnableRevertMap(mapEditor);
}

static void mapEditorDisableRevertMap(FrameworkWindow *mapEditor) {
  mapEditorMenuDisableRevertMap(mapEditor);
}

void mapEditorRefreshRevertMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(data->saveStatus == SAVED) {
    mapEditorDisableRevertMap(mapEditor);
  } else {
    mapEditorEnableRevertMap(mapEditor);
  }
}

void mapEditorRefreshTitle(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  SetWindowTitles(mapEditor->intuitionWindow, data->title, (STRPTR)-1);
}

static int saveMapRequester(FrameworkWindow *mapEditor) {
  char title[96];
  MapEditorData *data = mapEditor->data;

  sprintf(title, "Save Map %s", data->map->name);
  return spawnMapRequester(mapEditor, title);
}

BOOL mapEditorSaveMapAs(FrameworkWindow *mapEditor) {
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

  mapEditorDataSetMapNum(mapEditor->data, selected - 1);

  mapEditorDataSetSaveStatus(mapEditor->data, SAVED);

  projectDataUpdateMapName(projectData, selected - 1, data->map);

  return TRUE;
}

BOOL mapEditorSaveMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  if(!data->mapNum) {
    return mapEditorSaveMapAs(mapEditor);
  } else {
    projectDataOverwriteMap(projectData, data->map, data->mapNum - 1);
    projectDataUpdateMapName(projectData, data->mapNum - 1, data->map);
    mapEditorDataSetSaveStatus(mapEditor->data, SAVED);
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

void mapEditorRevertMap(FrameworkWindow *mapEditor) {
  if(confirmRevertMap(mapEditor)) {
    FrameworkWindow *projectWindow = mapEditor->parent;
    MapEditorData *data = mapEditor->data;
    mapEditor->closed = 1;
    projectWindowOpenMapNum(projectWindow, data->mapNum - 1);
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
    case 0: return FALSE;                       /* cancel */
    case 1: return mapEditorSaveMap(mapEditor); /* save */
    case 2: return TRUE;                        /* don't save */
    default:
      fprintf(stderr, "unsavedMapEditorAlert: unknown response %d\n", response);
      return FALSE;
    }
}

BOOL ensureMapEditorSaved(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  return (BOOL)(data->saveStatus == SAVED || unsavedMapEditorAlert(mapEditor));
}

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

void mapEditorChooseTilesetClicked(FrameworkWindow *mapEditor) {
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
      projectWindowSelectTilesetPackage(projectWindow);
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

void mapEditorUpdateMapName(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  MapEditorGadgets *gadgets = &data->gadgets;
  struct StringInfo *stringInfo = gadgets->mapNameGadget->SpecialInfo;
  mapEditorDataSetMapName(data, stringInfo->Buffer);
}

void mapEditorChangeSongClicked(FrameworkWindow *mapEditor) {
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
      mapEditorDataSetSongRequester(data, songRequester);
      /* TODO: fix me */
      /* addWindowToSet(songRequester->window); */
    }
  } else {
    WindowToFront(data->songRequester->window->intuitionWindow);
  }
}

void mapEditorClearSongClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  mapEditorDataClearSong(data);
}

static void moveToMap(FrameworkWindow *mapEditor, int mapNum) {
  FrameworkWindow *projectWindow = mapEditor->parent;

  if(ensureMapEditorSaved(mapEditor)) {
    if(projectWindowOpenMapNum(projectWindow, mapNum - 1)) {
      mapEditor->closed = 1;
    }
  }
}

void mapEditorMapUpClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum - 16);
}

void mapEditorMapDownClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum + 16);
}

void mapEditorMapLeftClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum - 1);
}

void mapEditorMapRightClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  moveToMap(mapEditor, data->mapNum + 1);
}

static void openNewEntityBrowser(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  data->entityBrowser = newEntityBrowser(mapEditor, data->map, data->mapNum);
}

void mapEditorEntitiesClicked(FrameworkWindow *mapEditor) {
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
void mapEditorDrawEntities(FrameworkWindow *mapEditor) {
  int i;
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[0];
  struct RastPort *rport = mapEditor->intuitionWindow->RPort;

  for(i = 0; i < data->map->entityCnt; i++) {
    drawEntity(rport, entity, i);
    entity++;
  }
}

void mapEditorDrawMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  DrawImage(mapEditor->intuitionWindow->RPort, data->mapImages,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);
}

void mapEditorDrawPalette(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  DrawImage(mapEditor->intuitionWindow->RPort, data->paletteImages,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);
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
  mapEditorDataSetTileTo(data, row, col, data->selected);
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
  (MenuSpec*)        NULL, /* fill me in later */
  (RefreshFunction)  refreshMapEditor,
  (CanCloseFunction) ensureMapEditorSaved,
  (CloseFunction)    NULL,
  (ClickFunction)    handleMapEditorClick
};

BOOL isMapEditor(FrameworkWindow *window) {
  return (BOOL)(window->kind == &mapEditorKind);
}

static struct EasyStruct tilesetOutOfBoundsEasyStruct = {
  sizeof(struct EasyStruct),
  0,
  "Tileset Not In New Tileset Package",
  "This map had tileset %ld, which does not exist\nin the new package.\nThe tileset has been removed from this map.",
  "OK"
};

void mapEditorDrawEntity(FrameworkWindow *mapEditor, int entityNum) {
  MapEditorData *data = mapEditor->data;
  Entity *entity = &data->map->entities[entityNum];
  if(data->map->tilesetNum) {
    drawEntity(mapEditor->intuitionWindow->RPort, entity, entityNum);
  }
}

static void mapEditorClearTileDisplays(FrameworkWindow *mapEditor) {
  struct RastPort *rport = mapEditor->intuitionWindow->RPort;

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

static void mapEditorDrawTileDisplays(FrameworkWindow *mapEditor) {
  mapEditorDrawPalette(mapEditor);
  mapEditorDrawMap(mapEditor);
  mapEditorDrawEntities(mapEditor);
}

void mapEditorRefreshTileDisplays(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(data->map->tilesetNum) {
    mapEditorDrawTileDisplays(mapEditor);
  } else {
    mapEditorClearTileDisplays(mapEditor);
  }

/* TODO: this needs to go somewhere...
  also when the tileset package is loaded set you need to load the images in the data properly...

  if(data->map->tilesetNum - 1 < projectDataGetTilesetCount(projectData)) {
    } else {
      EasyRequest(mapEditor->intuitionWindow, &tilesetOutOfBoundsEasyStruct, NULL,
        data->map->tilesetNum - 1);
      mapEditorDataClearTileset(mapEditor->data);
    }
*/
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
  mapEditorRefreshTileDisplays(mapEditor);
}

/* TODO: when is this called - when you update a song in the song name editor... */
void mapEditorRefreshSong(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(data->map->songNum) {
    /* mapEditorSetSongUpdateUI(mapEditor, data->map->songNum - 1); */
  }
}

void mapEditorRedrawTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  if(data->map->tilesetNum) {
    redrawMapTile(mapEditor, row, col);
  }
}

static FrameworkWindow *newMapEditor(FrameworkWindow *parent) {
  FrameworkWindow *mapEditor;
  struct Gadget *gadgets;

  MapEditorData *data = newMapEditorData();
  if(!data) {
    fprintf(stderr, "newMapEditor: failed to allocate MapEditorData\n");
    goto error;
  }

  gadgets = initMapEditorGadgets(&data->gadgets);
  if(!gadgets) {
    fprintf(stderr, "newMapEditor: failed to create gadgets\n");
    goto error_freeData;
  }

  mapEditorKind.menuSpec = mapEditorMenuSpec;
  mapEditor = openChildWindow(parent, &mapEditorKind, gadgets);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditor: failed to open window\n");
    goto error_freeData;
  }

  refreshMapEditor(mapEditor);

  mapEditor->data = data;

  return mapEditor;

error_freeData:
  freeMapEditorData(data);
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

FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map *map, int mapNum) {
  Map *mapCopy;
  FrameworkWindow *mapEditor;
  MapEditorData *data;
  MapEditorGadgets *gadgets;

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
  gadgets = &data->gadgets;

  GT_SetGadgetAttrs(gadgets->mapNameGadget, mapEditor->intuitionWindow, NULL,
    GTST_String, map->name,
    TAG_END);

  data->map = mapCopy;

  if(data->map->tilesetNum) {
    /* TODO: there should be a way for new to do this */
    mapEditorDataInitImages(data);
    /* TODO: mapEditorSetTilesetUpdateUI(mapEditor, map->tilesetNum - 1); */
  }

  if(map->songNum) {
    mapEditorDataSetSong(mapEditor->data, map->songNum - 1);
  }

  mapEditorDataSetMapNum(mapEditor->data, mapNum);
  return mapEditor;

error_freeMap:
  free(mapCopy);
error:
  return NULL;
}
