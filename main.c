#include <proto/exec.h>

#include <proto/dos.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <proto/graphics.h>

#include <libraries/asl.h>
#include <proto/asl.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "MapEditor.h"
#include "TilesetPackage.h"

#define SCR_WIDTH  640
#define SCR_HEIGHT 512

static struct Library *intuition = NULL;

static struct NewScreen newScreen = {
	0,0,SCR_WIDTH,SCR_HEIGHT,2,
	0,3,
	HIRES|LACE,
	CUSTOMSCREEN,
	NULL,
	"FracIDE Map Editor",
	NULL,
	NULL
};	

static struct Window *projectWindow = NULL;
static struct NewWindow projectNewWindow = {
	0,0,SCR_WIDTH,SCR_HEIGHT,
	0xFF,0xFF,
	MENUPICK,
	BORDERLESS|BACKDROP,
	NULL,
	NULL,
	"Project",
	NULL,
	NULL,
	SCR_WIDTH,SCR_WIDTH,
	0xFFFF,0xFFFF,
	CUSTOMSCREEN
};

static struct NewMenu newMenu[] = {
	{ NM_TITLE, "Project", 0, 0, 0, 0 },
		{ NM_ITEM, "Select Tileset Package...",  0,  0, 0, 0 },
		{ NM_ITEM, NM_BARLABEL,                  0,  0, 0, 0 },
		{ NM_ITEM, "Quit",                      "Q", 0, 0, 0 },
	{ NM_TITLE, "Maps",   0, 0, 0, 0 },
		{ NM_ITEM, "New Map", 0, 0, 0, 0 },
	{ NM_END,   NULL,      0, 0, 0, 0 }
};
static struct Menu *menu = NULL;

static struct EasyStruct noTilesetPackageLoadedEasyStruct = {
	sizeof(struct EasyStruct),
	0,
	"No Tileset Package Loaded",
	"Cannot choose tileset when no tileset package has been loaded.",
	"Select Tileset Package...|Cancel"
};

static int running = 0;
static long sigMask = 0;
static MapEditor *firstMapEditor = NULL;
static TilesetPackage *tilesetPackage = NULL;

static void addWindowToSigMask(struct Window *window) {
	sigMask |= 1L << window->UserPort->mp_SigBit;
}

static void removeWindowFromSigMask(struct Window *window) {
	sigMask &= ~(1L << window->UserPort->mp_SigBit);
}

static void addToMapEditorList(MapEditor *mapEditor) {
	mapEditor->next = firstMapEditor;
	firstMapEditor = mapEditor;
}

static void removeFromMapEditorList(MapEditor *mapEditor) {
	if(mapEditor->next) {
		mapEditor->next->prev = mapEditor->prev;
	}
	if(mapEditor->prev) {
		mapEditor->prev->next = mapEditor->next;
	} else {
		mapEditor = mapEditor->next;
	}
}

/* TODO: return something so we can retry on error? */
static void loadTilesetPackage(char *dir, char *file) {
	int pathSize = strlen(dir) + strlen(file);
	char *buffer = malloc(pathSize+1);
	if(!buffer) {
		goto error;
	}
	strcpy(buffer, dir);
	if(!AddPart(buffer, file, pathSize)) {
		goto freeBuffer;
	}

	tilesetPackage = tilesetPackageLoadFromFile(buffer);
	if(!tilesetPackage) {
		/* TODO: display an error */
	}
freeBuffer:
	free(buffer);
error:
	return;
}

static void selectTilesetPackage(void) {
	struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
		ASL_Hail, "Select Tileset Package",
		ASL_Window, projectWindow,
		TAG_END);
	if(request) {
		if(AslRequest(request, NULL)) {
			loadTilesetPackage(request->rf_Dir, request->rf_File);
		}
		FreeAslRequest(request);
	}
}

static void handleProjectMenuPick(UWORD itemNum, UWORD subNum) {
	switch(itemNum) {
		case 0: selectTilesetPackage(); break;
		case 2: running = 0; break;
	}
}

static void newMap(void) {
	MapEditor *mapEditor = newMapEditor();
	addToMapEditorList(mapEditor);
	addWindowToSigMask(mapEditor->window);
}

static void handleMapsMenuPick(UWORD itemNum, UWORD subNum) {
	switch(itemNum) {
		case 0: newMap(); break;
	}
}

static void handleMenuPick(UWORD menuNumber) {
	UWORD menuNum = MENUNUM(menuNumber);
	UWORD itemNum = ITEMNUM(menuNumber);
	UWORD subNum  = SUBNUM(menuNumber);
	switch(menuNum) {
		case 0: handleProjectMenuPick(itemNum, subNum); break;
		case 1: handleMapsMenuPick(itemNum, subNum); break;
	}
}

static void handleMenuPicks(UWORD menuNumber) {
	struct MenuItem *item = NULL;
	while(running && menuNumber != MENUNULL) {
		handleMenuPick(menuNumber);
		item = ItemAddress(menu, menuNumber);
		menuNumber = item->NextSelect;
	}
}

static void handleProjectMessage(struct IntuiMessage* msg) {
	switch(msg->Class) {
		case IDCMP_MENUPICK:
			handleMenuPick(msg->Code);
	}
}

static void handleProjectMessages(void) {
	struct IntuiMessage *msg;
	while(msg = (struct IntuiMessage*)GetMsg(projectWindow->UserPort)) {
		handleProjectMessage(msg);
		ReplyMsg((struct Message*)msg);
	}
}

static void handleChooseTilesetClicked(MapEditor *mapEditor) {
	int choice = EasyRequest(
		mapEditor->window,
		&noTilesetPackageLoadedEasyStruct,
		NULL);
	if(choice) {
		selectTilesetPackage();
	}
}

static void handleMapEditorGadgetUp
(MapEditor *mapEditor, struct Gadget *gadget) {
	switch(gadget->GadgetID) {
	case CHOOSE_TILESET_ID:
		handleChooseTilesetClicked(mapEditor);
		break;
	}
}

static void handleMapEditorMessage(MapEditor *mapEditor, struct IntuiMessage *msg) {
	switch(msg->Class) {
	case IDCMP_CLOSEWINDOW:
		mapEditor->closed = 1;
		break;
	case IDCMP_REFRESHWINDOW:
		GT_BeginRefresh(mapEditor->window);
		refreshMapEditor(mapEditor);
		GT_EndRefresh(mapEditor->window, TRUE);
		break;
	case IDCMP_GADGETUP:
		handleMapEditorGadgetUp(mapEditor, (struct Gadget*)msg->IAddress);
		break;
	}
}

static void handleMapEditorMessages(MapEditor *mapEditor) {
	struct IntuiMessage *msg = NULL;
	while(msg = GT_GetIMsg(mapEditor->window->UserPort)) {
		handleMapEditorMessage(mapEditor, msg);
		GT_ReplyIMsg(msg);
	}
}

static void handleAllMapEditorMessages(long signalSet) {
	MapEditor *i = firstMapEditor;
	while(i) {
		if(1L << i->window->UserPort->mp_SigBit & signalSet) {
			handleMapEditorMessages(i);
		}
		i = i->next;
	}
}

static void closeDeadMapEditors(void) {
	MapEditor *i = firstMapEditor;
	while(i) {
		MapEditor *next = i->next;
		if(i->closed) {
			if(i->next) {
				i->next->prev = i->prev;
			}
			if(i->prev) {
				i->prev->next = i->next;
			} else {
				firstMapEditor = next;
			}
			removeWindowFromSigMask(i->window);
			closeMapEditor(i);
		}
		i = next;
	}
}

static void mainLoop(void) {
	long signalSet = 0;
	running = 1;
	while(running) {
		signalSet = Wait(sigMask);
		if(1L << projectWindow->UserPort->mp_SigBit & signalSet) {
			handleProjectMessages();
		}
		handleAllMapEditorMessages(signalSet);
		closeDeadMapEditors();
	}
}

static void closeAllMapEditors(void) {
	MapEditor *i = firstMapEditor;
	while(i) {
		MapEditor *next = i->next;
		removeWindowFromSigMask(i->window);
		closeMapEditor(i);
		i = next;
	}
}

static void initPalette(struct ViewPort *viewport) {
	int i;
	UBYTE c = 15;
	for(i = 0; i < 4; i++, c -= 5) {
		SetRGB4(viewport, i, c, c, c);
	}
}

int main(void) {
	int retCode;
	
	intuition =	OpenLibrary("intuition.library", 0);
	if(!intuition) {
		retCode = -1;
		goto done;
	}

	screen = OpenScreen(&newScreen);
	if(!screen) {
		retCode = -2;
		goto closeIntuition;
	}
	
	initPalette(&screen->ViewPort);

	initMapEditorScreen();

	projectNewWindow.Screen = screen;
	projectWindow = OpenWindow(&projectNewWindow);
	if(!projectWindow) {
		retCode = -3;
		goto closeScreen;
	}
	addWindowToSigMask(projectWindow);

	vi = GetVisualInfo(screen, TAG_END);
	if(!vi) {
		retCode = -4;
		goto closeWindow;
	}

	initMapEditorVi();

	menu = CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
	if(!menu) {
		retCode = -5;
		goto freeVisualInfo;
	}

	if(!LayoutMenus(menu, vi, TAG_END)) {
		retCode = -6;
		goto freeMenu;

	}

	SetMenuStrip(projectWindow, menu);

	ActivateWindow(projectWindow);

	mainLoop();
	
	retCode = 0;
closeAllMapEditors:
	closeAllMapEditors();
freeTilesetPackage:
	free(tilesetPackage);
clearMenu:
	ClearMenuStrip(projectWindow);
freeMenu:
	FreeMenus(menu);
freeVisualInfo:
	FreeVisualInfo(vi);
closeWindow:
	removeWindowFromSigMask(projectWindow);
	CloseWindow(projectWindow);
closeScreen:
	CloseScreen(screen);
closeIntuition:
	CloseLibrary(intuition);
done:
	return retCode;
}
