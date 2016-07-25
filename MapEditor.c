#include "MapEditor.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <stdlib.h>

#include "globals.h"

#define MAP_EDITOR_WIDTH  400
#define MAP_EDITOR_HEIGHT 320

static struct NewWindow mapEditorNewWindow = {
	40,40,MAP_EDITOR_WIDTH, MAP_EDITOR_HEIGHT,
	0xFF, 0xFF,
	CLOSEWINDOW,
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

MapEditor *newMapEditor(void) {
	MapEditor *mapEditor = malloc(sizeof(MapEditor));
	if(!mapEditor) {
		return NULL;
	}

	mapEditor->window = OpenWindow(&mapEditorNewWindow);
	if(!mapEditor->window) {
		free(mapEditor);
		return NULL;
	}

	mapEditor->prev =   NULL;
	mapEditor->next =   NULL;
	mapEditor->closed = 0;

	return mapEditor;
}

void closeMapEditor(MapEditor *mapEditor) {
	CloseWindow(mapEditor->window);
	free(mapEditor);
}
