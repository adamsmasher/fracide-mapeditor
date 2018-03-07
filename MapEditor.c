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

#include "framework/menubuild.h"
#include "framework/screen.h"

#include "easystructs.h"
#include "EntityBrowser.h"
#include "globals.h"
#include "map.h"
#include "workspace.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

#define MAP_EDITOR_WIDTH  536
#define MAP_EDITOR_HEIGHT 384

static void newMapMenuItemClicked(FrameworkWindow *window) {
  newMap();
}

static void openMapMenuItemClicked(FrameworkWindow *window) {
  openMap();
}

static void saveMapMenuItemClicked(FrameworkWindow *window) {
  MapEditor *mapEditor = window->data;
  saveMap(mapEditor);
}

static void saveMapAsMenuItemClicked(FrameworkWindow *window) {
  MapEditor *mapEditor = window->data;
  saveMapAs(mapEditor);
}

static int confirmRevertMap(MapEditor *mapEditor) {
  return EasyRequest(
    mapEditor->window->intuitionWindow,
    &confirmRevertMapEasyStruct,
    NULL,
    mapEditor->mapNum - 1,
    mapEditor->map->name);
}

static void revertMap(MapEditor *mapEditor) {
  if(confirmRevertMap(mapEditor)) {
    mapEditor->closed = 1;
    openMapNum(mapEditor->mapNum - 1);
  }
}

static void revertMenuItemClicked(FrameworkWindow *window) {
  MapEditor *mapEditor = window->data;
  revertMap(mapEditor);
}

static void closeMenuItemClicked(FrameworkWindow *window) {
  MapEditor *mapEditor = window->data;
  if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
    mapEditor->closed = 1;
  }
}

static MenuSectionSpec newSection =
  { { "New", "N", MENU_ITEM_ENABLED, newMapMenuItemClicked },
    END_SECTION };

static MenuSectionSpec openSection =
  { { "New", "N", MENU_ITEM_ENABLED, openMapMenuItemClicked },
    END_SECTION };

static MenuSectionSpec saveSection =
  { { "Save",       "S",         MENU_ITEM_ENABLED,  saveMapMenuItemClicked   },
    { "Save As...", "A",         MENU_ITEM_ENABLED,  saveMapAsMenuItemClicked },
    { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED, revertMenuItemClicked    },
  END_SECTION };

static MenuSectionSpec closeSection =
  { { "Close", "Q", MENU_ITEM_ENABLED, closeMenuItemClicked },
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

static struct Menu *menu = NULL;

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

static void refreshMapEditorWindow(FrameworkWindow *mapEditorWindow) {
  drawBorders(mapEditorWindow->intuitionWindow->RPort);
}

static void refreshMapEditor(MapEditor *mapEditor) {
  refreshMapEditorWindow(mapEditor->window);
}

static WindowKind mapEditorWindowKind = {
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
  (MenuSpec*)        NULL,
  (RefreshFunction)  refreshMapEditorWindow,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL
};

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

static struct NewGadget mapNameNewGadget = {
  MAP_NAME_LEFT,  MAP_NAME_TOP,
  MAP_NAME_WIDTH, MAP_NAME_HEIGHT,
  "Map Name:",
  &Topaz80,
  MAP_NAME_ID,
  PLACETEXT_LEFT,
  NULL, /* visual info, filled in later */
  NULL  /* user data */
};

static struct NewGadget currentTilesetNewGadget = {
  CURRENT_TILESET_LEFT,  CURRENT_TILESET_TOP,
  CURRENT_TILESET_WIDTH, CURRENT_TILESET_HEIGHT,
  "Current Tileset",
  &Topaz80,
  CURRENT_TILESET_ID,
  PLACETEXT_ABOVE,
  NULL, /* visual info, filled in later */
  NULL  /* user data */
};

static struct NewGadget chooseTilesetNewGadget = {
  CHOOSE_TILESET_LEFT, CHOOSE_TILESET_TOP,
  CHOOSE_TILESET_WIDTH, CHOOSE_TILESET_HEIGHT,
  "Choose Tileset...",
  &Topaz80,
  CHOOSE_TILESET_ID,
  PLACETEXT_IN,
  NULL, /* visual info, filled in later */
  NULL  /* user data */
};

static struct NewGadget tilesetScrollNewGadget = {
  TILESET_SCROLL_LEFT,  TILESET_SCROLL_TOP,
  TILESET_SCROLL_WIDTH, TILESET_SCROLL_HEIGHT,
  NULL, /* no text */
  NULL,
  TILESET_SCROLL_ID,
  0,    /* flags */
  NULL, /* visual info, filled in later */
  NULL  /* user data */
};

static struct NewGadget songNameNewGadget = {
  SONG_NAME_LEFT,  SONG_NAME_TOP,
  SONG_NAME_WIDTH, SONG_NAME_HEIGHT,
  "Soundtrack:",
  &Topaz80,
  SONG_NAME_LABEL_ID,
  PLACETEXT_LEFT,
  NULL,  /* visual info, filled in later */
  NULL   /* user data */
};

static struct NewGadget songChangeNewGadget = {
  SONG_CHANGE_LEFT,  SONG_CHANGE_TOP,
  SONG_CHANGE_WIDTH, SONG_CHANGE_HEIGHT,
  "Change...",
  &Topaz80,
  SONG_CHANGE_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget songClearNewGadget = {
  SONG_CLEAR_LEFT,  SONG_CLEAR_TOP,
  SONG_CLEAR_WIDTH, SONG_CLEAR_HEIGHT,
  "X",
  &Topaz80,
  SONG_CLEAR_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget mapLeftNewGadget = {
  MAP_LEFT_LEFT,  MAP_LEFT_TOP,
  MAP_LEFT_WIDTH, MAP_LEFT_HEIGHT,
  "<",
  &Topaz80,
  MAP_LEFT_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget mapRightNewGadget = {
  MAP_RIGHT_LEFT,  MAP_RIGHT_TOP,
  MAP_RIGHT_WIDTH, MAP_RIGHT_HEIGHT,
  ">",
  &Topaz80,
  MAP_RIGHT_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget mapUpNewGadget = {
  MAP_UP_LEFT,  MAP_UP_TOP,
  MAP_UP_WIDTH, MAP_UP_HEIGHT,
  "^",
  &Topaz80,
  MAP_UP_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget mapDownNewGadget = {
  MAP_DOWN_LEFT,  MAP_DOWN_TOP,
  MAP_DOWN_WIDTH, MAP_DOWN_HEIGHT,
  "v",
  &Topaz80,
  MAP_DOWN_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget entitiesNewGadget = {
  ENTITIES_LEFT,  ENTITIES_TOP,
  ENTITIES_WIDTH, ENTITIES_HEIGHT,
  "Entities...",
  &Topaz80,
  ENTITIES_ID,
  0,
  NULL,
  NULL
};

static struct NewGadget *allNewGadgets[] = {
  &mapNameNewGadget,
  &currentTilesetNewGadget,
  &chooseTilesetNewGadget,
  &tilesetScrollNewGadget,
  &songNameNewGadget,
  &songChangeNewGadget,
  &songClearNewGadget,
  &mapLeftNewGadget,
  &mapRightNewGadget,
  &mapUpNewGadget,
  &mapDownNewGadget,
  &entitiesNewGadget,
  NULL
};

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

static struct EasyStruct tilesetOutOfBoundsEasyStruct = {
  sizeof(struct EasyStruct),
  0,
  "Tileset Not In New Tileset Package",
  "This map had tileset %ld, which does not exist\nin the new package.\nThe tileset has been removed from this map.",
  "OK"
};

static void createMapEditorGadgets(MapEditor *mapEditor) {
  struct Gadget *gad;
  struct Gadget *glist = NULL;

  gad = CreateContext(&glist);

  gad = CreateGadget(TEXT_KIND, gad, &currentTilesetNewGadget,
    GTTX_Text, "N/A",
    GTTX_Border, TRUE,
    TAG_END);
  mapEditor->tilesetNameGadget = gad;
	
  gad = CreateGadget(BUTTON_KIND, gad, &chooseTilesetNewGadget, TAG_END);

  gad = CreateGadget(SCROLLER_KIND, gad, &tilesetScrollNewGadget,
    PGA_Freedom, LORIENT_VERT,
    GA_Disabled, TRUE,
    TAG_END);
	
  gad = CreateGadget(STRING_KIND, gad, &mapNameNewGadget,
    TAG_END);
  mapEditor->mapNameGadget = gad;

  gad = CreateGadget(TEXT_KIND, gad, &songNameNewGadget,
    GTTX_Text, "N/A",
    GTTX_Border, TRUE,
    TAG_END);
  mapEditor->songNameGadget = gad;

  gad = CreateGadget(BUTTON_KIND, gad, &songChangeNewGadget, TAG_END);
  gad = CreateGadget(BUTTON_KIND, gad, &songClearNewGadget, TAG_END);

  gad = CreateGadget(BUTTON_KIND, gad, &mapLeftNewGadget,
    GA_Disabled, TRUE,
    TAG_END);
  mapEditor->leftGadget = gad;

  gad = CreateGadget(BUTTON_KIND, gad, &mapRightNewGadget,
    GA_Disabled, TRUE,
    TAG_END);
  mapEditor->rightGadget = gad;

  gad = CreateGadget(BUTTON_KIND, gad, &mapUpNewGadget,
    GA_Disabled, TRUE,
    TAG_END);
  mapEditor->upGadget = gad;

  gad = CreateGadget(BUTTON_KIND, gad, &mapDownNewGadget,
    GA_Disabled, TRUE,
    TAG_END);
  mapEditor->downGadget = gad;

  gad = CreateGadget(BUTTON_KIND, gad, &entitiesNewGadget, TAG_END);

  if(gad) {
    mapEditor->gadgets = glist;
  } else {
    mapEditor->tilesetNameGadget = NULL;
    mapEditor->mapNameGadget = NULL;
    mapEditor->songNameGadget = NULL;
    mapEditor->leftGadget = NULL;
    mapEditor->rightGadget = NULL;
    mapEditor->upGadget = NULL;
    mapEditor->downGadget = NULL;
    FreeGadgets(glist);
  }
}

static void initMapEditorPaletteImages(MapEditor *mapEditor) {
  int top, left, row, col;
  struct Image *i = mapEditor->paletteImages;
  UWORD *imageData = mapEditor->imageData;

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
  mapEditor->paletteImages[31].NextImage = NULL;
}

static void initMapEditorMapImages(MapEditor *mapEditor) {
  int top, left, row, col;
  struct Image *i = mapEditor->mapImages;
  UWORD *imageData = mapEditor->imageData;

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
  mapEditor->mapImages[89].NextImage = NULL;
}

static void closeAttachedTilesetRequester(MapEditor *mapEditor) {
  if(mapEditor->tilesetRequester) {
    closeTilesetRequester(mapEditor->tilesetRequester);
    mapEditor->tilesetRequester = NULL;
  }
}

static void closeAttachedSongRequester(MapEditor *mapEditor) {
  if(mapEditor->songRequester) {
    freeSongRequester(mapEditor->songRequester);
    mapEditor->songRequester = NULL;
  }
}

static void closeAttachedEntityBrowser(MapEditor *mapEditor) {
  if(mapEditor->entityBrowser) {
    freeEntityBrowser(mapEditor->entityBrowser);
    mapEditor->entityBrowser = NULL;
  }
}

void closeMapEditor(MapEditor *mapEditor) {
  /* TODO: these should all be taken care of by the framework
  closeAttachedTilesetRequester(mapEditor);
  closeAttachedSongRequester(mapEditor);
  closeAttachedEntityBrowser(mapEditor);
  ClearMenuStrip(mapEditor->window);
  CloseWindow(mapEditor->window); */
  FreeGadgets(mapEditor->gadgets);
  FreeMem(mapEditor->imageData, IMAGE_DATA_SIZE);
  free(mapEditor->map);
  free(mapEditor);
}

static void attachTilesetRequesterToMapEditor
(MapEditor *mapEditor, TilesetRequester *tilesetRequester) {
  mapEditor->tilesetRequester = tilesetRequester;
}

static void attachSongRequesterToMapEditor
(MapEditor *mapEditor, SongRequester *songRequester) {
  mapEditor->songRequester = songRequester;
}

static void attachEntityBrowserToMapEditor
(MapEditor *mapEditor, EntityBrowser *entityBrowser) {
  mapEditor->entityBrowser = entityBrowser;
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

void mapEditorDrawEntity(MapEditor *mapEditor, Entity *entity, int entityNum) {
  if(mapEditor->map->tilesetNum) {
    drawEntity(mapEditor->window->intuitionWindow->RPort, entity, entityNum);
  }
}

/* IN A DREAM WORLD: store the IntuiTexts in the map editor and render them all at once */
static void drawEntities(MapEditor *mapEditor) {
  int i;
  Entity           *entity;
  struct RastPort  *rport;

  rport = mapEditor->window->intuitionWindow->RPort;

  entity = &mapEditor->map->entities[0];
  for(i = 0; i < mapEditor->map->entityCnt; i++) {
    drawEntity(rport, entity, i);
    entity++;
  }
}

static void mapEditorSetTilesetUpdateUI(MapEditor *mapEditor, UWORD tilesetNumber) {
  ProjectWindowData *parentData = mapEditor->window->parent->data;
  TilesetPackage *tilesetPackage = NULL /* TODO: fix me parentData->tilesetPackage */;

  GT_SetGadgetAttrs(mapEditor->tilesetNameGadget, mapEditor->window->intuitionWindow, NULL,
    GTTX_Text, tilesetPackage->tilesetPackageFile.tilesetNames[tilesetNumber],
    TAG_END);

  copyScaledTileset(
    (UWORD*)tilesetPackage->tilesetPackageFile.tilesetImgs[tilesetNumber],
    mapEditor->imageData);

  DrawImage(mapEditor->window->intuitionWindow->RPort, mapEditor->paletteImages,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);

  DrawImage(mapEditor->window->intuitionWindow->RPort, mapEditor->mapImages,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);

  drawEntities(mapEditor);
}

static void mapEditorClearTilesetUI(MapEditor *mapEditor) {
  struct RastPort *rport = mapEditor->window->intuitionWindow->RPort;

  GT_SetGadgetAttrs(mapEditor->tilesetNameGadget, mapEditor->window->intuitionWindow, NULL,
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

static void updateMapEditorTitle(MapEditor *mapEditor) {
  char unsaved = mapEditor->saved ? '\0' : '*';
  if(mapEditor->mapNum) {
    sprintf(mapEditor->title, "Map %d%c", mapEditor->mapNum - 1, unsaved);
  } else {
    sprintf(mapEditor->title, "Map Editor%c", unsaved);
  }
  SetWindowTitles(mapEditor->window->intuitionWindow, mapEditor->title, (STRPTR)-1);
}

void mapEditorSetSaveStatus(MapEditor *mapEditor, int status) {
  mapEditor->saved = status;
  updateMapEditorTitle(mapEditor);
}

void mapEditorRefreshTileset(MapEditor *mapEditor) {
  ProjectWindowData *parentData = mapEditor->window->parent->data;
  TilesetPackage *tilesetPackage = NULL/* TODO: fix me parentData->tilesetPackage */;

  if(mapEditor->map->tilesetNum) {
    if(mapEditor->map->tilesetNum - 1 < tilesetPackage->tilesetPackageFile.tilesetCnt) {
       mapEditorSetTilesetUpdateUI(mapEditor, mapEditor->map->tilesetNum - 1);
    } else {
      mapEditorClearTilesetUI(mapEditor);
      EasyRequest(mapEditor->window->intuitionWindow, &tilesetOutOfBoundsEasyStruct, NULL,
        mapEditor->map->tilesetNum - 1);
      mapEditor->map->tilesetNum = 0;
      mapEditorSetSaveStatus(mapEditor, UNSAVED);
    }
  }
}

void mapEditorSetTileset(MapEditor *mapEditor, UWORD tilesetNumber) {
  mapEditor->map->tilesetNum = tilesetNumber + 1;
  mapEditorSetTilesetUpdateUI(mapEditor, tilesetNumber);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void mapEditorSetSongUpdateUI(MapEditor *mapEditor, UWORD songNumber) {
  GT_SetGadgetAttrs(mapEditor->songNameGadget, mapEditor->window->intuitionWindow, NULL,
    GTTX_Text, projectDataGetSongName(mapEditor->window->parent->data, songNumber),
    TAG_END);
}

static void mapEditorClearSongUpdateUI(MapEditor *mapEditor) {
  GT_SetGadgetAttrs(mapEditor->songNameGadget, mapEditor->window->intuitionWindow, NULL,
    GTTX_Text, "N/A",
    TAG_END);
}

void mapEditorSetSong(MapEditor *mapEditor, UWORD songNumber) {
  mapEditor->map->songNum = songNumber + 1;
  mapEditorSetSongUpdateUI(mapEditor, songNumber);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

void mapEditorRefreshSong(MapEditor *mapEditor) {
  if(mapEditor->map->songNum) {
     mapEditorSetSongUpdateUI(mapEditor, mapEditor->map->songNum - 1);
  }
}

static void mapEditorClearSong(MapEditor *mapEditor) {
  mapEditor->map->songNum = 0;
  mapEditorClearSongUpdateUI(mapEditor);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void redrawPaletteTile(MapEditor *mapEditor, unsigned int tile) {
  struct Image *image = &mapEditor->paletteImages[tile];
  struct Image *next = image->NextImage;
  image->NextImage = NULL;
  DrawImage(mapEditor->window->intuitionWindow->RPort, image,
    TILESET_BORDER_LEFT,
    TILESET_BORDER_TOP);
  image->NextImage = next;
}

static void redrawMapTile(MapEditor *mapEditor, unsigned int tile) {
  struct Image *image = &mapEditor->mapImages[tile];
  struct Image *next = image->NextImage;
  int entity_i;

  image->NextImage = NULL;
  DrawImage(mapEditor->window->intuitionWindow->RPort, image,
    MAP_BORDER_LEFT,
    MAP_BORDER_TOP);
  image->NextImage = next;

  if(entity_i = mapFindEntity(mapEditor->map, tile / 10, tile % 10)) {
    entity_i--;
    drawEntity(mapEditor->window->intuitionWindow->RPort, &mapEditor->map->entities[entity_i], entity_i);
  }
}

void mapEditorRedrawTile(MapEditor *mapEditor, int row, int col) {
  if(mapEditor->map->tilesetNum) {
    redrawMapTile(mapEditor, row * 10 + col);
  }
}

static void mapEditorSetTileTo(MapEditor *mapEditor, unsigned int tile, UBYTE to) {
  mapEditor->map->tiles[tile] = to;
  mapEditor->mapImages[tile].ImageData = mapEditor->imageData + (to << 7);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void mapEditorSetTile(MapEditor *mapEditor, unsigned int tile) {
  mapEditorSetTileTo(mapEditor, tile, mapEditor->selected);
  redrawMapTile(mapEditor, tile);
}

void mapEditorSetMapNum(MapEditor *mapEditor, UWORD mapNum) {
  mapEditor->mapNum = mapNum + 1;

  if(mapNum < 16) {
    GT_SetGadgetAttrs(mapEditor->upGadget, mapEditor->window->intuitionWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);
  } else {
    GT_SetGadgetAttrs(mapEditor->upGadget, mapEditor->window->intuitionWindow, NULL,
      GA_Disabled, FALSE,
      TAG_END);
  }

  if(mapNum >= 112) {
    GT_SetGadgetAttrs(mapEditor->downGadget, mapEditor->window->intuitionWindow, NULL,
      GA_Disabled, TRUE,
      TAG_END);
    } else {
      GT_SetGadgetAttrs(mapEditor->downGadget, mapEditor->window->intuitionWindow, NULL,
        GA_Disabled, FALSE,
        TAG_END);
    }

    if(mapNum % 16 == 0) {
      GT_SetGadgetAttrs(mapEditor->leftGadget, mapEditor->window->intuitionWindow, NULL,
        GA_Disabled, TRUE,
        TAG_END);
    } else {
      GT_SetGadgetAttrs(mapEditor->leftGadget, mapEditor->window->intuitionWindow, NULL,
        GA_Disabled, FALSE,
        TAG_END);
    }

    if(mapNum % 16 == 15) {
      GT_SetGadgetAttrs(mapEditor->rightGadget, mapEditor->window->intuitionWindow, NULL,
        GA_Disabled, TRUE,
        TAG_END);
    } else {
      GT_SetGadgetAttrs(mapEditor->rightGadget, mapEditor->window->intuitionWindow, NULL,
        GA_Disabled, FALSE,
        TAG_END);
    }

    updateMapEditorTitle(mapEditor);
}

static void initMapEditorVi(void) {
  struct NewGadget **i = allNewGadgets;
  void *vi = getGlobalVi();
  if(!vi) {
    fprintf(stderr, "initMapEditorVi: failed to get global vi\n");
  }

  while(*i) {
    (*i)->ng_VisualInfo = vi;
    i++;
  }
}

static MapEditor *newMapEditor(void) {
  MapEditor *mapEditor = malloc(sizeof(MapEditor));
  if(!mapEditor) {
    fprintf(stderr, "newMapEditor: failed to allocate mapEditor\n");
    goto error;
  }

  createMapEditorGadgets(mapEditor);
  if(!mapEditor->gadgets) {
    fprintf(stderr, "newMapEditor: failed to create gadgets\n");
    goto error_freeEditor;
  }
  mapEditorWindowKind.newWindow.FirstGadget = mapEditor->gadgets;

  mapEditor->imageData = AllocMem(IMAGE_DATA_SIZE, MEMF_CHIP);
  if(!mapEditor->imageData) {
    fprintf(stderr, "newMapEditor: failed to allocate image data\n");
    goto error_freeGadgets;
  }
  initMapEditorPaletteImages(mapEditor);
  initMapEditorMapImages(mapEditor);

  mapEditor->window = openWindowOnGlobalScreen(&mapEditorWindowKind);
  if(!mapEditor->window) {
    fprintf(stderr, "newMapEditor: failed to open window\n");
    goto error_freeImageData;
  }

  refreshMapEditor(mapEditor);

  mapEditor->prev             = NULL;
  mapEditor->next             = NULL;
  mapEditor->tilesetRequester = NULL;
  mapEditor->songRequester    = NULL;
  mapEditor->entityBrowser    = NULL;
  mapEditor->closed           = 0;
  mapEditor->selected         = -1;

  mapEditor->window->data = mapEditor;

  return mapEditor;

error_freeImageData:
  FreeMem(mapEditor->imageData, IMAGE_DATA_SIZE);
error_freeGadgets:
  FreeGadgets(mapEditor->gadgets);
error_freeEditor:
  free(mapEditor);
error:
    return NULL;
}

MapEditor *newMapEditorNewMap(void) {
  Map *map;
  MapEditor *mapEditor;

  map = allocMap();
  if(!map) {
    goto error;
  }

  mapEditor = newMapEditor();
  if(!mapEditor) {
    goto error_freeMap;
  }

  mapEditor->map = map;
  mapEditor->mapNum = 0;
  mapEditorSetSaveStatus(mapEditor, SAVED);
  return mapEditor;

error_freeMap:
    free(map);
error:
    return NULL;
}

MapEditor *newMapEditorWithMap(Map *map, int mapNum) {
  Map *mapCopy;
  MapEditor *mapEditor;

  mapCopy = map ? copyMap(map) : allocMap();

  if(!mapCopy) {
    goto error;
  }

  mapEditor = newMapEditor();
  if(!mapEditor) {
    goto error_freeMap;
  }

  GT_SetGadgetAttrs(mapEditor->mapNameGadget, mapEditor->window->intuitionWindow, NULL,
    GTST_String, map->name,
    TAG_END);

  mapEditor->map = mapCopy;

  if(map->tilesetNum) {
    int i;
    for(i = 0; i < MAP_TILES_WIDE * MAP_TILES_HIGH; i++) {
      mapEditorSetTileTo(mapEditor, i, map->tiles[i]);
    }
    mapEditorSetTilesetUpdateUI(mapEditor, map->tilesetNum - 1);
  }

  if(map->songNum) {
    mapEditorSetSong(mapEditor, map->songNum - 1);
  }

  mapEditorSetMapNum(mapEditor, mapNum);
  mapEditorSetSaveStatus(mapEditor, SAVED);
  return mapEditor;

error_freeMap:
  free(mapCopy);
error:
  return NULL;
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

static unsigned int mapEditorGetMapTileClicked(WORD x, WORD y) {
  unsigned int row = y;
  unsigned int col = x;

  row -= MAP_BORDER_TOP;
  col -= MAP_BORDER_LEFT;

  row >>= 5;
  col >>= 5;

  return (row * 10) + col;
}

static void mapEditorSetSelected(MapEditor *mapEditor, unsigned int selected) {
  long row;
  long col;

  if(mapEditor->selected >= 0) {
    redrawPaletteTile(mapEditor, mapEditor->selected);
  }

  mapEditor->selected = (int)selected;

  row = selected >> 2;
  col = selected & 0x03;

  DrawBorder(mapEditor->window->intuitionWindow->RPort, &tileBorder,
    TILESET_BORDER_LEFT + (col * 32),
    TILESET_BORDER_TOP  + (row * 32));
}

static void updateMapEditorMapName(MapEditor *mapEditor) {
  struct StringInfo *stringInfo =
    (struct StringInfo*)mapEditor->mapNameGadget->SpecialInfo;
  strcpy(mapEditor->map->name, stringInfo->Buffer);
  mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

void enableMapRevert(MapEditor *mapEditor) {
  OnMenu(mapEditor->window->intuitionWindow, REVERT_MAP_MENU_ITEM);
}

void disableMapRevert(MapEditor *mapEditor) {
  OffMenu(mapEditor->window->intuitionWindow, REVERT_MAP_MENU_ITEM);
}

static void openNewEntityBrowser(MapEditor *mapEditor) {
  char title[32];
  EntityBrowser *entityBrowser;

  if(mapEditor->mapNum) {
    sprintf(title, "Entities (Map %d)", mapEditor->mapNum - 1);
  } else {
    strcpy(title, "Entities");
  }

  entityBrowser = newEntityBrowser(title, mapEditor->map->entities, mapEditor->map->entityCnt);
  if(!entityBrowser) {
    fprintf(stderr, "openNewEntityBrowser: couldn't create new entity browser\n");
    goto error;
  }

  attachEntityBrowserToMapEditor(mapEditor, entityBrowser);
  /* TODO: fix me */
  /* addWindowToSet(entityBrowser->window); */

  done:
    return;

  error:
    return;
}

static void handleChooseTilesetClicked(MapEditor *mapEditor) {
  TilesetRequester *tilesetRequester;
  char title[32];
  FrameworkWindow *projectWindow = mapEditor->window->parent;
  ProjectWindowData *parentData = projectWindow->data;
  TilesetPackage *tilesetPackage = NULL /* TODO: fix me parentData->tilesetPackage */;

  if(mapEditor->tilesetRequester) {
    WindowToFront(mapEditor->tilesetRequester->window->intuitionWindow);
    goto done;
  }

  if(!tilesetPackage) {
    int choice = EasyRequest(
      mapEditor->window->intuitionWindow,
      &noTilesetPackageLoadedEasyStruct,
      NULL);

    if(choice) {
      selectTilesetPackage(projectWindow);
    }
  }

  /* even after giving the user the opportunity to set the tileset
     package, we need to be sure they did so... */
  if(!tilesetPackage) {
    goto done;
  }

  if(mapEditor->mapNum) {
    sprintf(title, "Choose Tileset For Map %d", mapEditor->mapNum - 1);
  } else {
    strcpy(title, "Choose Tileset");
  }

  tilesetRequester = newTilesetRequester(title);
  if(!tilesetRequester) {
    fprintf(stderr, "handleChooseTilesetClicked: couldn't make requester\n");
    goto error;
  }

  attachTilesetRequesterToMapEditor(mapEditor, tilesetRequester);
  /* TODO: fix me */
  /* addWindowToSet(tilesetRequester->window); */
done:
  return;
error:
  return;
}

static void handleClearSongClicked(MapEditor *mapEditor) {
  mapEditorClearSong(mapEditor);
}

static void handleChangeSongClicked(MapEditor *mapEditor) {
  if(!mapEditor->songRequester) {
    char title[32];
    SongRequester *songRequester;

    if(mapEditor->mapNum) {
      sprintf(title, "Change Soundtrack For Map %d", mapEditor->mapNum - 1);
    } else {
      strcpy(title, "Change Soundtrack");
    }

    songRequester = newSongRequester(title);
    if(songRequester) {
      attachSongRequesterToMapEditor(mapEditor, songRequester);
      /* TODO: fix me */
      /* addWindowToSet(songRequester->window); */
    }
  } else {
    WindowToFront(mapEditor->songRequester->window->intuitionWindow);
  }
}

static void moveToMap(MapEditor *mapEditor, int mapNum) {
  if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
    if(openMapNum(mapNum - 1)) {
      mapEditor->closed = 1;
    }
  }
}

static void handleMapUp(MapEditor *mapEditor) {
  moveToMap(mapEditor, mapEditor->mapNum - 16);
}

static void handleMapDown(MapEditor *mapEditor) {
  moveToMap(mapEditor, mapEditor->mapNum + 16);
}

static void handleMapLeft(MapEditor *mapEditor) {
  moveToMap(mapEditor, mapEditor->mapNum - 1);
}

static void handleMapRight(MapEditor *mapEditor) {
  moveToMap(mapEditor, mapEditor->mapNum + 1);
}

static void handleEntitiesClicked(MapEditor *mapEditor) {
  if(!mapEditor->entityBrowser) {
    openNewEntityBrowser(mapEditor);
  } else {
    WindowToFront(mapEditor->entityBrowser->window->intuitionWindow);
  }
}

static void handleMapEditorGadgetUp
(MapEditor *mapEditor, struct Gadget *gadget) {
  switch(gadget->GadgetID) {
    case CHOOSE_TILESET_ID:
      handleChooseTilesetClicked(mapEditor);
      break;
    case MAP_NAME_ID:
      updateMapEditorMapName(mapEditor);
      break;
    case SONG_CHANGE_ID:
      handleChangeSongClicked(mapEditor);
      break;
    case SONG_CLEAR_ID:
      handleClearSongClicked(mapEditor);
      break;
    case MAP_LEFT_ID:
      handleMapLeft(mapEditor);
      break;
    case MAP_RIGHT_ID:
      handleMapRight(mapEditor);
      break;
    case MAP_UP_ID:
      handleMapUp(mapEditor);
      break;
    case MAP_DOWN_ID:
      handleMapDown(mapEditor);
      break;
    case ENTITIES_ID:
      handleEntitiesClicked(mapEditor);
      break;
    }
}

static void handleMapEditorPaletteClick(MapEditor *mapEditor, WORD x, WORD y) {
  int tile = mapEditorGetPaletteTileClicked(x, y);
  mapEditorSetSelected(mapEditor, tile);
}

static void handleMapEditorMapClick(MapEditor *mapEditor, WORD x, WORD y) {
  unsigned int tile = mapEditorGetMapTileClicked(x, y);
  mapEditorSetTile(mapEditor, tile);
}

static void handleMapEditorClick(MapEditor *mapEditor, WORD x, WORD y) {
  if(mapEditor->map->tilesetNum) {
    if(mapEditorClickInPalette(x, y)) {
      handleMapEditorPaletteClick(mapEditor, x, y);
    } else if(mapEditorClickInMap(x, y)) {
      handleMapEditorMapClick(mapEditor, x, y);
    }
  }
}

static void handleMapEditorMessage(MapEditor *mapEditor, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
      if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
        mapEditor->closed = 1;
      }
      break;
    case IDCMP_REFRESHWINDOW:
      GT_BeginRefresh(mapEditor->window->intuitionWindow);
      refreshMapEditor(mapEditor);
      GT_EndRefresh(mapEditor->window->intuitionWindow, TRUE);
      break;
    case IDCMP_GADGETUP:
      handleMapEditorGadgetUp(mapEditor, (struct Gadget*)msg->IAddress);
      break;
    case IDCMP_MOUSEBUTTONS:
      handleMapEditorClick(mapEditor, msg->MouseX, msg->MouseY);
      break;
  }
}

static void handleMapEditorMessages(MapEditor *mapEditor) {
  struct IntuiMessage *msg = NULL;
  while(msg = GT_GetIMsg(mapEditor->window->intuitionWindow->UserPort)) {
    handleMapEditorMessage(mapEditor, msg);
    GT_ReplyIMsg(msg);
  }
}
