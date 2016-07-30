#include "MapEditor.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>

#include "globals.h"

#define MAP_EDITOR_WIDTH  512
#define MAP_EDITOR_HEIGHT 320

static struct NewWindow mapEditorNewWindow = {
	40,40,MAP_EDITOR_WIDTH, MAP_EDITOR_HEIGHT,
	0xFF, 0xFF,
	CLOSEWINDOW|REFRESHWINDOW|GADGETUP,
	WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
	NULL,
	NULL,
	"Map Editor",
	NULL,
	NULL,
	MAP_EDITOR_WIDTH,MAP_EDITOR_HEIGHT,
	MAP_EDITOR_WIDTH,MAP_EDITOR_HEIGHT,
	CUSTOMSCREEN
};

/* TODO: get the font from the system preferences */
struct TextAttr Topaz80 = { "topaz.font", 8, 0, 0 };

/* TODO: adjust based on titlebar height */
#define CURRENT_TILESET_LEFT   344
#define CURRENT_TILESET_TOP    36
#define CURRENT_TILESET_WIDTH  144
#define CURRENT_TILESET_HEIGHT 12

#define CHOOSE_TILESET_LEFT    CURRENT_TILESET_LEFT
#define CHOOSE_TILESET_TOP     CURRENT_TILESET_TOP + CURRENT_TILESET_HEIGHT
#define CHOOSE_TILESET_HEIGHT  12
#define CHOOSE_TILESET_WIDTH   CURRENT_TILESET_WIDTH

/* TODO: adjust based on screen */
#define MAP_BORDER_LEFT   20
#define MAP_BORDER_TOP    20
#define MAP_BORDER_WIDTH  320
#define MAP_BORDER_HEIGHT 288

static struct NewGadget currentTilesetNewGadget = {
	CURRENT_TILESET_LEFT, CURRENT_TILESET_TOP,
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

static struct NewGadget *allNewGadgets[] = {
	&currentTilesetNewGadget,
	&chooseTilesetNewGadget,
	NULL
};

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

struct Gadget *createMapEditorGadgets(void) {
	struct Gadget *gad;
	struct Gadget *glist = NULL;

	gad = CreateContext(&glist);

	gad = CreateGadget(TEXT_KIND, gad, &currentTilesetNewGadget,
		GTTX_Text, "N/A",
		GTTX_Border, TRUE,
		TAG_END);
	
	gad = CreateGadget(BUTTON_KIND, gad, &chooseTilesetNewGadget, TAG_END);

	if(gad) {
		return glist;
	} else {
		FreeGadgets(glist);
		return NULL;
	}
}

void refreshMapEditor(MapEditor *mapEditor) {
	DrawBorder(mapEditor->window->RPort, &mapBorder,
		MAP_BORDER_LEFT, MAP_BORDER_TOP);
}

MapEditor *newMapEditor(void) {
	MapEditor *mapEditor = malloc(sizeof(MapEditor));
	if(!mapEditor) {
		goto error;
	}

	mapEditor->gadgets = createMapEditorGadgets();
	if(!mapEditor->gadgets) {
		goto error_freeEditor;
	}
	mapEditorNewWindow.FirstGadget = mapEditor->gadgets;

	mapEditor->window = OpenWindow(&mapEditorNewWindow);
	if(!mapEditor->window) {
		goto error_freeGadgets;
	}

	GT_RefreshWindow(mapEditor->window, NULL);
	refreshMapEditor(mapEditor);

	mapEditor->prev =   NULL;
	mapEditor->next =   NULL;
	mapEditor->closed = 0;

	return mapEditor;

error_freeGadgets:
	FreeGadgets(mapEditor->gadgets);
error_freeEditor:
	free(mapEditor);
error:
	return NULL;
}

void closeMapEditor(MapEditor *mapEditor) {
	CloseWindow(mapEditor->window);
	FreeGadgets(mapEditor->gadgets);
	free(mapEditor);
}
