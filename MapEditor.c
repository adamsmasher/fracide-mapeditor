#include "MapEditor.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>

#include "globals.h"

#define MAP_EDITOR_WIDTH  400
#define MAP_EDITOR_HEIGHT 320

static struct NewWindow mapEditorNewWindow = {
	40,40,MAP_EDITOR_WIDTH, MAP_EDITOR_HEIGHT,
	0xFF, 0xFF,
	CLOSEWINDOW|REFRESHWINDOW,
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
#define CURRENT_TILESET_LEFT   230
#define CURRENT_TILESET_TOP    36
#define CURRENT_TILESET_WIDTH  144
#define CURRENT_TILESET_HEIGHT 12
#define CURRENT_TILESET_ID     0

static struct NewGadget currentTilesetNewGadget = {
	CURRENT_TILESET_LEFT, CURRENT_TILESET_TOP,
	CURRENT_TILESET_WIDTH, CURRENT_TILESET_HEIGHT,
	"Current Tileset",
	&Topaz80, /* font */
	CURRENT_TILESET_ID,
	PLACETEXT_ABOVE,
	NULL, /* visual info, filled in later */
	NULL  /* user data */
};

void initMapEditorScreen(void) {
	mapEditorNewWindow.Screen = screen;
}

void initMapEditorVi(void) {
	currentTilesetNewGadget.ng_VisualInfo = vi;
}

struct Gadget *createMapEditorGadgets(void) {
	struct Gadget *gad;
	struct Gadget *glist = NULL;

	gad = CreateContext(&glist);

	gad = CreateGadget(TEXT_KIND, gad, &currentTilesetNewGadget,
		GTTX_Text, "N/A",
		GTTX_Border, TRUE,
		TAG_END);

	if(gad) {
		return glist;
	} else {
		FreeGadgets(glist);
		return NULL;
	}
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
