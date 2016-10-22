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

#include "map.h"
#include "TilesetRequester.h"
#include "globals.h"

#define MAP_EDITOR_WIDTH  520
#define MAP_EDITOR_HEIGHT 356

static struct NewMenu newMenu[] = {
  { NM_TITLE, "Map", 0, 0, 0, 0 },
    { NM_ITEM, "New",        "N", 0,               0, 0 },
    { NM_ITEM, NM_BARLABEL,  0,   0,               0, 0 },
    { NM_ITEM, "Open...",    "O", 0,               0, 0 },
    { NM_ITEM, NM_BARLABEL,  0,   0,               0, 0 },
    { NM_ITEM, "Save",       "S", 0,               0, 0 },
    { NM_ITEM, "Save As...", "A", 0,               0, 0 },
    { NM_ITEM, "Revert",     0,   NM_ITEMDISABLED, 0, 0 },
    { NM_ITEM, NM_BARLABEL,  0,   0,               0, 0 },
    { NM_ITEM, "Close",      "Q", 0,               0, 0 },
  { NM_END,   NULL,   0, 0, 0, 0 }
};
static struct Menu *menu = NULL;

static struct NewWindow mapEditorNewWindow = {
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
};

#define TILE_WIDTH  16
#define TILE_HEIGHT 16

/* TODO: adjust based on titlebar height */
#define CURRENT_TILESET_LEFT   352
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

/* TODO: adjust based on screen */
#define MAP_BORDER_LEFT   20
#define MAP_BORDER_TOP    37
#define MAP_BORDER_WIDTH  MAP_TILES_ACROSS * TILE_WIDTH  * 2 + 2
#define MAP_BORDER_HEIGHT MAP_TILES_HIGH   * TILE_HEIGHT * 2 + 2

#define MAP_NAME_LEFT   (MAP_BORDER_LEFT  + 80)
#define MAP_NAME_TOP    18
#define MAP_NAME_WIDTH  (MAP_BORDER_WIDTH - 81)
#define MAP_NAME_HEIGHT 14

#define TILESET_BORDER_LEFT   CURRENT_TILESET_LEFT
#define TILESET_BORDER_TOP    TILESET_SCROLL_TOP + 1
#define TILESET_BORDER_WIDTH  TILE_WIDTH * TILESET_PALETTE_TILES_ACROSS * 2 + 2
#define TILESET_BORDER_HEIGHT TILESET_SCROLL_HEIGHT

#define SONG_NAME_LEFT   (MAP_BORDER_LEFT + 95)
#define SONG_NAME_TOP    (MAP_BORDER_TOP + MAP_BORDER_HEIGHT + 4)
#define SONG_NAME_WIDTH  (MAP_BORDER_WIDTH - 170)
#define SONG_NAME_HEIGHT 14

#define SONG_CHANGE_LEFT   (SONG_NAME_LEFT + SONG_NAME_WIDTH - 2)
#define SONG_CHANGE_TOP    SONG_NAME_TOP
#define SONG_CHANGE_WIDTH  76
#define SONG_CHANGE_HEIGHT 14

#define IMAGE_DATA_SIZE (TILES_PER_SET * 256)

/* TODO: add gadgets for moving between maps quickly */

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

static struct NewGadget *allNewGadgets[] = {
  &mapNameNewGadget,
  &currentTilesetNewGadget,
  &chooseTilesetNewGadget,
  &tilesetScrollNewGadget,
  &songNameNewGadget,
  &songChangeNewGadget,
  NULL
};

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

void initMapEditorScreen(void) {
  mapEditorNewWindow.Screen = screen;
}

void initMapEditorVi(void) {
    struct NewGadget **i = allNewGadgets;
    while(*i) {
        (*i)->ng_VisualInfo = vi;
        i++;
    }
}

struct Menu *initMapEditorMenu(void) {
    menu = CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
    if(!menu) {
        goto error;
    }

    if(!LayoutMenus(menu, vi, TAG_END)) {
        goto error_freeMenu;
    }

    return menu;
	
error_freeMenu:
    FreeMenus(menu);
error:
    return NULL;
}

void freeMapEditorMenu(void) {
    FreeMenus(menu);
}

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

    if(gad) {
        mapEditor->gadgets = glist;
    } else {
        mapEditor->tilesetNameGadget = NULL;
        mapEditor->mapNameGadget = NULL;
        mapEditor->songNameGadget = NULL;
        FreeGadgets(glist);
    }
}

static void drawBorders(struct RastPort *rport) {
    DrawBorder(rport, &mapBorder, MAP_BORDER_LEFT, MAP_BORDER_TOP);
    DrawBorder(rport, &tilesetBorder,
        TILESET_BORDER_LEFT, TILESET_BORDER_TOP);
}

void refreshMapEditor(MapEditor *mapEditor) {
	drawBorders(mapEditor->window->RPort);
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

void closeMapEditor(MapEditor *mapEditor) {
    closeAttachedTilesetRequester(mapEditor);
    closeAttachedSongRequester(mapEditor);
    ClearMenuStrip(mapEditor->window);
    CloseWindow(mapEditor->window);
    FreeGadgets(mapEditor->gadgets);
    FreeMem(mapEditor->imageData, IMAGE_DATA_SIZE);
    free(mapEditor->map);
    free(mapEditor);
}

void attachTilesetRequesterToMapEditor
(MapEditor *mapEditor, TilesetRequester *tilesetRequester) {
    mapEditor->tilesetRequester = tilesetRequester;
}

void attachSongRequesterToMapEditor
(MapEditor *mapEditor, SongRequester *songRequester) {
    mapEditor->songRequester = songRequester;
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

static void mapEditorSetTilesetUpdateUI(MapEditor *mapEditor, UWORD tilesetNumber) {
    GT_SetGadgetAttrs(mapEditor->tilesetNameGadget, mapEditor->window, NULL,
        GTTX_Text, tilesetPackage->tilesetPackageFile.tilesetNames[tilesetNumber],
        TAG_END);

    copyScaledTileset(
        (UWORD*)tilesetPackage->tilesetPackageFile.tilesetImgs[tilesetNumber],
        mapEditor->imageData);

    DrawImage(mapEditor->window->RPort, mapEditor->paletteImages,
        TILESET_BORDER_LEFT,
        TILESET_BORDER_TOP);

    DrawImage(mapEditor->window->RPort, mapEditor->mapImages,
        MAP_BORDER_LEFT,
        MAP_BORDER_TOP);
}

void mapEditorSetTileset(MapEditor *mapEditor, UWORD tilesetNumber) {
    mapEditor->map->tilesetNum = tilesetNumber + 1;
    mapEditorSetTilesetUpdateUI(mapEditor, tilesetNumber);
    mapEditor->saved = 0;
}

static void mapEditorSetSongUpdateUI(MapEditor *mapEditor, UWORD songNumber) {
    GT_SetGadgetAttrs(mapEditor->songNameGadget, mapEditor->window, NULL,
        GTTX_Text, project.songNameStrs[songNumber],
        TAG_END);
}

void mapEditorSetSong(MapEditor *mapEditor, UWORD songNumber) {
    mapEditor->map->songNum = songNumber + 1;
    mapEditorSetSongUpdateUI(mapEditor, songNumber);
    mapEditor->saved = 0;
}

static void redrawPaletteTile(MapEditor *mapEditor, unsigned int tile) {
    struct Image *image = &mapEditor->paletteImages[tile];
    struct Image *next = image->NextImage;
    image->NextImage = NULL;
    DrawImage(mapEditor->window->RPort, image,
        TILESET_BORDER_LEFT,
        TILESET_BORDER_TOP);
    image->NextImage = next;
}

static void redrawMapTile(MapEditor *mapEditor, unsigned int tile) {
    struct Image *image = &mapEditor->mapImages[tile];	struct Image *next = image->NextImage;
    image->NextImage = NULL;
    DrawImage(mapEditor->window->RPort, image,
        MAP_BORDER_LEFT,
        MAP_BORDER_TOP);
    image->NextImage = next;
}

static void mapEditorSetTileTo(MapEditor *mapEditor, unsigned int tile, UBYTE to) {
    mapEditor->map->tiles[tile] = to;
    mapEditor->mapImages[tile].ImageData = mapEditor->imageData + (to << 7);
    redrawMapTile(mapEditor, tile);
    mapEditor->saved = 0;
}

void mapEditorSetTile(MapEditor *mapEditor, unsigned int tile) {
    mapEditorSetTileTo(mapEditor, tile, mapEditor->selected);
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
    mapEditorNewWindow.FirstGadget = mapEditor->gadgets;

    mapEditor->imageData = AllocMem(IMAGE_DATA_SIZE, MEMF_CHIP);
    if(!mapEditor->imageData) {
        fprintf(stderr, "newMapEditor: failed to allocate image data\n");
        goto error_freeGadgets;
    }
    initMapEditorPaletteImages(mapEditor);
    initMapEditorMapImages(mapEditor);

    mapEditor->window = OpenWindow(&mapEditorNewWindow);
    if(!mapEditor->window) {
        fprintf(stderr, "newMapEditor: failed to open window\n");
        goto error_freeImageData;
    }

    SetMenuStrip(mapEditor->window, menu);

    GT_RefreshWindow(mapEditor->window, NULL);
    refreshMapEditor(mapEditor);

    mapEditor->prev             = NULL;
    mapEditor->next             = NULL;
    mapEditor->tilesetRequester = NULL;
    mapEditor->songRequester    = NULL;
    mapEditor->closed           = 0;
    mapEditor->selected         = -1;

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

/* TODO: put something indicating save status in title bar */
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
    mapEditor->saved = 1;
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

    GT_SetGadgetAttrs(mapEditor->mapNameGadget, mapEditor->window, NULL,
        GTST_String, map->name,
        TAG_END);

    mapEditor->map = mapCopy;

    if(map->tilesetNum) {
        int i;
        mapEditorSetTilesetUpdateUI(mapEditor, map->tilesetNum - 1);
        for(i = 0; i < MAP_TILES_WIDE * MAP_TILES_HIGH; i++) {
            mapEditorSetTileTo(mapEditor, i, map->tiles[i]);
        }
    }

    if(map->songNum) {
        mapEditorSetSong(mapEditor, map->songNum - 1);
    }

    mapEditor->mapNum = mapNum + 1;
    mapEditor->saved = 1;
    return mapEditor;

error_freeMap:
    free(mapCopy);
error:
    return NULL;
}

int mapEditorClickInPalette(WORD x, WORD y) {
    return ((x > TILESET_BORDER_LEFT                        ) &&
            (x < TILESET_BORDER_LEFT + TILESET_BORDER_WIDTH ) &&
            (y > TILESET_BORDER_TOP                         ) &&
            (y < TILESET_BORDER_TOP  + TILESET_BORDER_HEIGHT));
}

int mapEditorClickInMap(WORD x, WORD y) {
    return ((x > MAP_BORDER_LEFT                    ) &&
            (x < MAP_BORDER_LEFT + MAP_BORDER_WIDTH ) &&
            (y > MAP_BORDER_TOP                     ) &&
            (y < MAP_BORDER_TOP  + MAP_BORDER_HEIGHT));
}

unsigned int mapEditorGetPaletteTileClicked(WORD x, WORD y) {
    unsigned int row = y;
    unsigned int col = x;

    row -= TILESET_BORDER_TOP;
    col -= TILESET_BORDER_LEFT;

    row >>= 5;
    col >>= 5;

    return (row << 2) + col;
}

unsigned int mapEditorGetMapTileClicked(WORD x, WORD y) {
    unsigned int row = y;
    unsigned int col = x;

    row -= MAP_BORDER_TOP;
    col -= MAP_BORDER_LEFT;

    row >>= 5;
    col >>= 5;

    return (row * 10) + col;
}

void mapEditorSetSelected(MapEditor *mapEditor, unsigned int selected) {
    long row;
    long col;

    if(mapEditor->selected >= 0) {
        redrawPaletteTile(mapEditor, mapEditor->selected);
    }

    mapEditor->selected = (int)selected;

    row = selected >> 2;
    col = selected & 0x03;

    DrawBorder(mapEditor->window->RPort, &tileBorder,
        TILESET_BORDER_LEFT + (col * 32),
        TILESET_BORDER_TOP  + (row * 32));
}

void updateMapEditorMapName(MapEditor *mapEditor) {
    struct StringInfo *stringInfo =
        (struct StringInfo*)mapEditor->mapNameGadget->SpecialInfo;
    strcpy(mapEditor->map->name, stringInfo->Buffer);
    mapEditor->saved = 0;
}
