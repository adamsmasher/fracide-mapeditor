#include "MapEditor.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <intuition/gadgetclass.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EasyStructs.h"
#include "MapEditorConstants.h"
#include "MapEditorData.h"
#include "MapEditorGadgets.h"
#include "MapEditorMenu.h"
#include "MapRequester.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"

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
  if(mapEditorDataIsSaved(mapEditor->data)) {
    mapEditorDisableRevertMap(mapEditor);
  } else {
    mapEditorEnableRevertMap(mapEditor);
  }
}

void mapEditorRefreshNavigationButtons(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  MapEditorGadgets *gadgets = mapEditor->gadgets->data;
  struct Window *window = mapEditor->intuitionWindow;
  BOOL upDisabled;
  BOOL downDisabled;
  BOOL leftDisabled;
  BOOL rightDisabled;

  if(mapEditorDataHasMapNum(data)) {
    UWORD mapNum = mapEditorDataGetMapNum(data);
    upDisabled    = (BOOL)(mapNum < 16);
    downDisabled  = (BOOL)(mapNum >= 112);
    leftDisabled  = (BOOL)(mapNum % 16 == 0);
    rightDisabled = (BOOL)(mapNum % 16 == 15);
  } else {
    upDisabled = downDisabled = leftDisabled = rightDisabled = TRUE;
  }

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
}

void mapEditorRefreshTitle(FrameworkWindow *mapEditor) {
  /* we have to cast away const because amiga is bad */
  char *title = (char*)mapEditorDataGetTitle(mapEditor->data);
  SetWindowTitles(mapEditor->intuitionWindow, title, (STRPTR)-1);
}

static int saveMapRequester(FrameworkWindow *mapEditor) {
  char title[96];

  sprintf(title, "Save Map %s", mapEditorDataGetMapName(mapEditor->data));
  return spawnMapRequester(mapEditor, title);
}

BOOL mapEditorSaveMapAs(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  ProjectWindowData *projectData = mapEditor->parent->data;

  int selected = saveMapRequester(mapEditor);
  if(!selected) {
    return FALSE;
  }
  selected--;

  if(!projectDataHasMap(projectData, selected)) {
    if(!mapEditorDataSaveMapAs(data, selected)) {
      fprintf(stderr, "mapEditorSaveMapAs: failed to save map\n");
      goto error;
    }
  } else {
    const char *mapToOverwrite = projectDataGetMapName(projectData, selected);
    int response = EasyRequest(
      mapEditor->intuitionWindow,
      &saveIntoFullSlotEasyStruct,
      NULL,
      selected,
      mapToOverwrite);
    if(response) {
      mapEditorDataSaveMapAs(data, selected);
    } else {
      return FALSE;
    }
  }

  return TRUE;
error:
  return FALSE;
}

BOOL mapEditorSaveMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(mapEditorDataHasMapNum(data)) {
    return mapEditorDataSaveMap(data);
  } else {
    return mapEditorSaveMapAs(mapEditor);
  }
}

static int confirmRevertMap(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  return EasyRequest(
    mapEditor->intuitionWindow,
    &confirmRevertMapEasyStruct,
    NULL,
    mapEditorDataGetMapNum(data),
    mapEditorDataGetMapName(data));
}

void mapEditorRevertMap(FrameworkWindow *mapEditor) {
  if(!mapEditorDataHasMapNum(mapEditor->data)) {
    fprintf(stderr, "mapEditorRevertMap: assertion failure; can't revert unsaved map\n");
    return;
  }

  if(confirmRevertMap(mapEditor)) {
    mapEditor->closed = TRUE;
    projectWindowOpenMapNum(mapEditor->parent, mapEditorDataGetMapNum(mapEditor->data));
  }
}

static BOOL unsavedMapEditorAlert(FrameworkWindow *mapEditor) {
  int response;
  MapEditorData *data = mapEditor->data;

  if(mapEditorDataHasMapNum(data)) {
    response = EasyRequest(
      mapEditor->intuitionWindow,
      &unsavedMapAlertEasyStructWithNum,
      NULL,
      mapEditorDataGetMapNum(data), 
      mapEditorDataGetMapName(data));
  } else {
    response = EasyRequest(
      mapEditor->intuitionWindow,
      &unsavedMapAlertEasyStructNoNum,
      NULL,
      mapEditorDataGetMapName(data));
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

BOOL mapEditorEnsureSaved(FrameworkWindow *mapEditor) {
  return (BOOL)(mapEditorDataIsSaved(mapEditor->data) || unsavedMapEditorAlert(mapEditor));
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

  if(mapEditorDataHasTilesetRequester(data)) {
    FrameworkWindow *tilesetRequester = mapEditorDataGetTilesetRequester(data);
    WindowToFront(tilesetRequester->intuitionWindow);
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

  if(mapEditorDataHasMapNum(data)) {
    sprintf(title, "Choose Tileset For Map %d", mapEditorDataGetMapNum(data));
  } else {
    strcpy(title, "Choose Tileset");
  }

  tilesetRequester = newTilesetRequester(title, mapEditor);
  if(!tilesetRequester) {
    fprintf(stderr, "handleChooseTilesetClicked: couldn't make requester\n");
    goto error;
  }

  mapEditorDataSetTilesetRequester(data, tilesetRequester);
done:
  return;
error:
  return;
}

void mapEditorUpdateMapName(FrameworkWindow *mapEditor) {
  const MapEditorGadgets *gadgets = mapEditor->gadgets->data;
  const struct StringInfo *stringInfo = gadgets->mapNameGadget->SpecialInfo;
  mapEditorDataSetMapName(mapEditor->data, stringInfo->Buffer);
}

void mapEditorRefreshMapName(FrameworkWindow *mapEditor) {
  MapEditorGadgets *gadgets = mapEditor->gadgets->data;

  GT_SetGadgetAttrs(gadgets->mapNameGadget, mapEditor->intuitionWindow, NULL,
    GTST_String, mapEditorDataGetMapName(mapEditor->data),
    TAG_END);
}

void mapEditorChangeSongClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(mapEditorDataHasSongRequester(data)) {
    SongRequester *songRequester = mapEditorDataGetSongRequester(data);
    WindowToFront(songRequester->window->intuitionWindow);
  } else {
    char title[32];
    SongRequester *songRequester;

    if(mapEditorDataHasMapNum(data)) {
      sprintf(title, "Change Soundtrack For Map %d", mapEditorDataGetMapNum(data));
    } else {
      strcpy(title, "Change Soundtrack");
    }

    songRequester = newSongRequester(title);
    if(songRequester) {
      mapEditorDataSetSongRequester(data, songRequester);
      /* TODO: fix me */
      /* addWindowToSet(songRequester->window); */
    } else {
      fprintf(stderr, "mapEditorChangeSongClicked: couldn't make SongRequester\n");
    }
  }
}

void mapEditorClearSongClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  mapEditorDataClearSong(data);
}

static void moveToMap(FrameworkWindow *mapEditor, int mapNum) {
  FrameworkWindow *projectWindow = mapEditor->parent;

  if(mapEditorEnsureSaved(mapEditor)) {
    if(projectWindowOpenMapNum(projectWindow, mapNum - 1)) {
      mapEditor->closed = TRUE;
    }
  }
}

void mapEditorMapUpClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(mapEditorDataHasMapNum(data)) {
    moveToMap(mapEditor, mapEditorDataGetMapNum(data) - 16);
  } else {
    fprintf(stderr, "mapEditorMapUpClicked: assert failure: no map number\n");
  }
}

void mapEditorMapDownClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(mapEditorDataHasMapNum(data)) {
    moveToMap(mapEditor, mapEditorDataGetMapNum(data) + 16);
  } else {
    fprintf(stderr, "mapEditorMapDownClicked: assert failure: no map number\n");
  }
}

void mapEditorMapLeftClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(mapEditorDataHasMapNum(data)) {
    moveToMap(mapEditor, mapEditorDataGetMapNum(data) - 1);
  } else {
    fprintf(stderr, "mapEditorMapLeftClicked: assert failure: no map number\n");
  }
}

void mapEditorMapRightClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  if(mapEditorDataHasMapNum(data)) {
    moveToMap(mapEditor, mapEditorDataGetMapNum(data) + 1);
  } else {
    fprintf(stderr, "mapEditorMapRightClicked: assert failure: no map number\n");   
  }
}

void mapEditorEntitiesClicked(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(mapEditorDataHasEntityBrowser(data)) {
    FrameworkWindow *entityBrowser = mapEditorDataGetEntityBrowser(data);
    WindowToFront(entityBrowser->intuitionWindow);
  } else {
    FrameworkWindow *entityBrowser;
    if(mapEditorDataHasMapNum(data)) {
      entityBrowser = newEntityBrowserWithMapNum(mapEditor, mapEditorDataGetMap(data), mapEditorDataGetMapNum(data) + 1);
    } else {
      entityBrowser = newEntityBrowserWithMapNum(mapEditor, mapEditorDataGetMap(data), 0);
    }
    if(entityBrowser) {
      mapEditorDataSetEntityBrowser(data, entityBrowser);
    } else {
      fprintf(stderr, "mapEditorEntitiesClicked: failed to open new entity browser\n");
    }
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
  struct Image *image = mapEditorDataGetPaletteImage(mapEditor->data, tile);
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

void mapEditorRefreshSelected(FrameworkWindow *mapEditor) {
  long row;
  long col;
  unsigned int selected = mapEditorDataGetSelected(mapEditor->data);

  row = selected >> 2;
  col = selected & 0x03;

  DrawBorder(mapEditor->intuitionWindow->RPort, &tileBorder,
    TILESET_BORDER_LEFT + (col * 32),
    TILESET_BORDER_TOP  + (row * 32));
}

void mapEditorRefreshSelectedFrom(FrameworkWindow *mapEditor, unsigned int from) {
  redrawPaletteTile(mapEditor, from);
  mapEditorRefreshSelected(mapEditor);
}

static void handleMapEditorPaletteClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  int tile = mapEditorGetPaletteTileClicked(x, y);
  mapEditorDataSetSelected(mapEditor->data, tile);
}

static void drawEntity(struct RastPort *rport, const Entity *entity, UWORD entityNum) {
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
  UWORD i;
  UWORD entityCount = mapEditorDataGetEntityCount(mapEditor->data);
  struct RastPort *rport = mapEditor->intuitionWindow->RPort;

  for(i = 0; i < entityCount; i++) {
    const Entity *entity = mapEditorDataGetEntity(mapEditor->data, i);
    drawEntity(rport, entity, i);
  }
}

void mapEditorDrawMap(FrameworkWindow *mapEditor) {
  DrawImage(mapEditor->intuitionWindow->RPort, mapEditorDataGetMapImages(mapEditor->data),
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);
}

void mapEditorDrawPalette(FrameworkWindow *mapEditor) {
  DrawImage(mapEditor->intuitionWindow->RPort, mapEditorDataGetPaletteImages(mapEditor->data),
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);
}

static void redrawMapTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  const Map *map = mapEditorDataGetMap(data);
  UBYTE tile = row * 10 + col;
  struct Image *image = mapEditorDataGetMapImage(data, tile);
  struct Image *next = image->NextImage;
  int entity_i;

  image->NextImage = NULL;
  DrawImage(mapEditor->intuitionWindow->RPort, image,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);
  image->NextImage = next;

  if(entity_i = mapFindEntity(map, row, col)) {
    entity_i--;
    drawEntity(mapEditor->intuitionWindow->RPort, &map->entities[entity_i], entity_i);
  }
}

static void mapEditorSetTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  MapEditorData *data = mapEditor->data;
  if(mapEditorDataHasSelected(data)) {
    mapEditorDataSetTileTo(data, row, col, mapEditorDataGetSelected(data));
  }
}

static void handleMapEditorMapClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  UBYTE col = mapEditorGetColClicked(x);
  UBYTE row = mapEditorGetRowClicked(y);
  mapEditorSetTile(mapEditor, row, col);
}

/* TODO: maybe we can use buttons for this... */
static void handleMapEditorClick(FrameworkWindow *mapEditor, WORD x, WORD y) {
  if(mapEditorDataHasTileset(mapEditor->data)) {
    if(mapEditorClickInPalette(x, y)) {
      handleMapEditorPaletteClick(mapEditor, x, y);
    } else if(mapEditorClickInMap(x, y)) {
      handleMapEditorMapClick(mapEditor, x, y);
    }
  }
}

static void freeMapEditor(FrameworkWindow *mapEditor) {
  freeMapEditorData(mapEditor->data);
  freeMapEditorGadgets(mapEditor->gadgets);
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
  (CanCloseFunction) mapEditorEnsureSaved,
  (CloseFunction)    freeMapEditor,
  (ClickFunction)    handleMapEditorClick
};

BOOL isMapEditor(FrameworkWindow *window) {
  return (BOOL)(window->kind == &mapEditorKind);
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

static void refreshTilesetRequesterChildren(FrameworkWindow *mapEditor) {
  FrameworkWindow *i = mapEditor->children;
  while(i) {
    if(isTilesetRequesterWindow(i)) {
      refreshTilesetRequesterList(i);
    }
    i = i->next;
  }
}

void mapEditorRefreshTileDisplays(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;

  if(mapEditorDataHasTileset(data)) {
    mapEditorDrawTileDisplays(mapEditor);
  } else {
    mapEditorClearTileDisplays(mapEditor);
  }

  refreshTilesetRequesterChildren(mapEditor);

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

void mapEditorRefreshTilesetName(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  MapEditorGadgets *gadgets = mapEditor->gadgets->data;
  const char *tilesetName;

  if(mapEditorDataHasTileset(data)) {
    UWORD tilesetNum = mapEditorDataGetTileset(data);
    tilesetName = projectDataGetTilesetName(mapEditor->parent->data, tilesetNum);
  } else {
    tilesetName = "N/A";
  }

  GT_SetGadgetAttrs(gadgets->tilesetNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, tilesetName,
    TAG_END);
}

void mapEditorRefreshSong(FrameworkWindow *mapEditor) {
  MapEditorData *data = mapEditor->data;
  MapEditorGadgets *gadgets = mapEditor->gadgets->data;
  const char *songName;

  if(mapEditorDataHasSong(data)) {
    UWORD songNum = mapEditorDataGetSong(data);
    songName = projectDataGetSongName(mapEditor->parent->data, songNum);
  } else {
    songName = "N/A";
  }

  GT_SetGadgetAttrs(gadgets->songNameGadget, mapEditor->intuitionWindow, NULL,
    GTTX_Text, songName,
    TAG_END);
}

void mapEditorRefreshTile(FrameworkWindow *mapEditor, UBYTE row, UBYTE col) {
  if(mapEditorDataHasTileset(mapEditor->data)) {
    redrawMapTile(mapEditor, row, col);
  }
}

static FrameworkWindow *newMapEditor(FrameworkWindow *parent, Map *map) {
  MapEditorData *data;
  WindowGadgets *gadgets;
  FrameworkWindow *mapEditor;

  data = newMapEditorData();
  if(!data) {
    fprintf(stderr, "newMapEditor: failed to allocate MapEditorData\n");
    goto error;
  }

  gadgets = newMapEditorGadgets();
  if(!gadgets) {
    fprintf(stderr, "newMapEditor: failed to create gadgets\n");
    goto error_freeData;
  }

  mapEditorKind.menuSpec = mapEditorMenuSpec;
  mapEditor = openChildWindow(parent, &mapEditorKind, gadgets);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditor: failed to open window\n");
    goto error_freeGadgets;
  }

  initMapEditorData(data, mapEditor, map);

  return mapEditor;

error_freeGadgets:
  freeMapEditorGadgets(gadgets);
error_freeData:
  freeMapEditorData(data);
error:
  return NULL;
}

FrameworkWindow *newMapEditorNewMap(FrameworkWindow *parent) {
  Map *map;
  FrameworkWindow *mapEditor;

  map = allocMap();
  if(!map) {
    fprintf(stderr, "newMapEditorNewMap: failed to allocate map\n");
    goto error;
  }

  sprintf(map->name, "Untitled");

  mapEditor = newMapEditor(parent, map);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditorNewMap: failed to create map editor\n");
    goto error_freeMap;
  }

  return mapEditor;

error_freeMap:
  free(map);
error:
  return NULL;
}

FrameworkWindow *newMapEditorWithMap(FrameworkWindow *parent, Map *map, UWORD mapNum) {
  Map *mapCopy;
  FrameworkWindow *mapEditor;

  mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "newMapEditorWithMap: failed to copy map\n");
    goto error;
  }

  mapEditor = newMapEditor(parent, map);
  if(!mapEditor) {
    fprintf(stderr, "newMapEditorWithMap: failed to create map editor\n");
    goto error_freeMap;
  }

  mapEditorDataSetMapNum(mapEditor->data, mapNum);

  return mapEditor;

error_freeMap:
  free(map);
error:
  return NULL;
}
