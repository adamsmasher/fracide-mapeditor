#include "TilesetRequester.h"

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>

#include "globals.h"

#define TILESET_REQUESTER_WIDTH  200
#define TILESET_REQUESTER_HEIGHT 336

static struct NewWindow tilesetRequesterNewWindow = {
	40, 40, TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_HEIGHT,
	0xFF, 0xFF,
	CLOSEWINDOW|REFRESHWINDOW|GADGETUP,
	WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
	NULL,
	NULL,
	"Choose Tileset", /* TODO: dynamically generate this based on tileset name */
	NULL,
	NULL,
	TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_HEIGHT,
	TILESET_REQUESTER_WIDTH, TILESET_REQUESTER_HEIGHT,
	CUSTOMSCREEN
};

void initTilesetRequesterScreen(void) {
	tilesetRequesterNewWindow.Screen = screen;
}

void initTilesetRequesterVi(void) {
}

TilesetRequester *newTilesetRequester(void) {
	TilesetRequester *tilesetRequester = malloc(sizeof(TilesetRequester));
	if(!tilesetRequester) {
		goto error;
	}

	tilesetRequester->gadgets = NULL;
	
	tilesetRequester->window = OpenWindow(&tilesetRequesterNewWindow);
	if(!tilesetRequester) {
		goto error;
	}

	/*GT_RefreshWindow(tilesetRequester->window, NULL);
	refreshTilesetRequester(tilesetRequester); */

	tilesetRequester->closed = 0;
	
	return tilesetRequester;
error:
	return NULL;
}

void closeTilesetRequester(TilesetRequester *tilesetRequester) {
	CloseWindow(tilesetRequester->window);
	free(tilesetRequester);
}

void refreshTilesetRequester(TilesetRequester *tilesetRequester) {
}
