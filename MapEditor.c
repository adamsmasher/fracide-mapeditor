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

	return glist;
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
