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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

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
		{ NM_ITEM, "New",                       "N", 0,               0, 0 },
		{ NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
		{ NM_ITEM, "Open...",                   "O", 0,               0, 0 },
		{ NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
		{ NM_ITEM, "Save",                      "S", 0,               0, 0 },
		{ NM_ITEM, "Save As...",                "A", 0,               0, 0 },
		{ NM_ITEM, "Revert",                     0,  NM_ITEMDISABLED, 0, 0 },
		{ NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
		{ NM_ITEM, "Select Tileset Package...",  0,  0,               0, 0 },
		{ NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
		{ NM_ITEM, "Quit",                      "Q", 0,               0, 0 },
	{ NM_TITLE, "Maps",   0, 0, 0, 0 },
		{ NM_ITEM, "New Map",     0, 0, 0, 0 },
		{ NM_ITEM, "Open Map...", 0, 0, 0, 0 },
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

static struct EasyStruct tilesetPackageLoadFailEasyStruct = {
	sizeof(struct EasyStruct),
	0,
	"Error Loading Tileset Package",
	"Could not load tileset package from\n%s.",
	"OK"
};

static struct EasyStruct projectLoadFailEasyStruct = {
	sizeof(struct EasyStruct),
	0,
	"Error Loading Project",
	"Could not load project from\n%s.",
	"OK"
};

static struct EasyStruct projectSaveFailEasyStruct = {
	sizeof(struct EasyStruct),
	0,
	"Error Saving Project",
	"Could not save project to \n%s.",
	"OK"
};

static int running = 0;
static long sigMask = 0;
static MapEditor *firstMapEditor = NULL;

static void addWindowToSigMask(struct Window *window) {
	sigMask |= 1L << window->UserPort->mp_SigBit;
}

static void removeWindowFromSigMask(struct Window *window) {
	sigMask &= ~(1L << window->UserPort->mp_SigBit);
}

static void addToMapEditorList(MapEditor *mapEditor) {
	mapEditor->next = firstMapEditor;
	if(firstMapEditor) {
		firstMapEditor->prev = mapEditor;
	}
	firstMapEditor = mapEditor;
}

static void removeFromMapEditorList(MapEditor *mapEditor) {
	if(mapEditor->next) {
		mapEditor->next->prev = mapEditor->prev;
	}
	if(mapEditor->prev) {
		mapEditor->prev->next = mapEditor->next;
	} else {
		firstMapEditor = mapEditor->next;
	}
}

/* TODO: return something so we can retry on error? */
static void loadTilesetPackage(char *dir, char *file) {
	TilesetPackage *newTilesetPackage;
	char buffer[TILESET_PACKAGE_PATH_SIZE];

	if(strlen(dir) >= sizeof(buffer)) {
		goto error;
	}

	strcpy(buffer, dir);
	if(!AddPart(buffer, file, TILESET_PACKAGE_PATH_SIZE)) {
		goto error;
	}

	newTilesetPackage = tilesetPackageLoadFromFile(buffer);
	if(!newTilesetPackage) {
		EasyRequest(projectWindow,
			&tilesetPackageLoadFailEasyStruct,
			NULL,
			buffer);
		goto error;
	}
	freeTilesetPackage(tilesetPackage);
	tilesetPackage = newTilesetPackage;
	strcpy(project.tilesetPackagePath, buffer);

error:
	return;
}

static void closeAllMapEditors(void) {
	MapEditor *i = firstMapEditor;
	while(i) {
		MapEditor *next = i->next;
		removeWindowFromSigMask(i->window);
		if(i->tilesetRequester) {
			removeWindowFromSigMask(i->tilesetRequester->window);
		}
		closeMapEditor(i);
		i = next;
	}
	firstMapEditor = NULL;
}

static void setProjectFilename(char *filename) {
	/* TODO: range check on filename length */
	ULONG revertMenuItem = SHIFTMENU(0) | SHIFTITEM(6);
	if(filename) {
		strcpy(projectFilename, filename);
		OnMenu(projectWindow, revertMenuItem);
	} else {
		projectFilename[0] = '\0';
		OffMenu(projectWindow, revertMenuItem);
	}	
}

static void newProject(void) {
	/* TODO: check for unsaved maps */
	closeAllMapEditors();
	freeTilesetPackage(tilesetPackage);
	tilesetPackage = NULL;
	freeProject(&project);
	initProject(&project);
	setProjectFilename(NULL);
}

static void openProjectFromAsl(char *dir, char *file) {
	Project myNewProject;
	size_t bufferLen = strlen(dir) + strlen(file) + 2;
	char *buffer = malloc(bufferLen);

	if(!buffer) {
		fprintf(
			stderr,
			"openProjectFromAsl: failed to allocate buffer "
			"(dir: %s) (file: %s)\n",
			dir  ? dir  : "NULL",
			file ? file : "NULL");
		goto done;
	}

	strcpy(buffer, dir);
	if(!AddPart(buffer, file, (ULONG)bufferLen)) {
		fprintf(
			stderr,
			"openProjectFromAsl: failed to add part "
			"(buffer: %s) (file: %s) (len: %d)\n",
			buffer ? buffer : "NULL",
			file   ? file   : "NULL",
			bufferLen);
		goto freeBuffer;
	}

	if(!loadProjectFromFile(buffer, &myNewProject)) {
		EasyRequest(projectWindow,
			&projectLoadFailEasyStruct,
			NULL,
			buffer);
		goto freeBuffer;
	}
	newProject();
	memcpy(&project, &myNewProject, sizeof(Project));
	setProjectFilename(buffer);

freeBuffer:
	free(buffer);
done:
	return;
}

static void openProject(void) {
	/* TODO: check for unsaved maps */
	struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
		ASL_Hail, "Open Project",
		ASL_Window, projectWindow,
		TAG_END);
	if(!request) {
		goto done;
	}

	if(AslRequest(request, NULL)) {
		openProjectFromAsl(request->rf_Dir, request->rf_File);
	}

	FreeAslRequest(request);
done:
	return;
}

static void saveProjectToAsl(char *dir, char *file) {
	size_t bufferLen = strlen(dir) + strlen(file) + 2;
	char *buffer = malloc(bufferLen);

	if(!buffer) {
		fprintf(
			stderr,
			"saveProjectToAsl: failed to allocate buffer "
			"(dir: %s) (file: %s)\n",
			dir  ? dir  : "NULL",
			file ? file : "NULL");
		goto done;
	}

	strcpy(buffer, dir);
	if(!AddPart(buffer, file, (ULONG)bufferLen)) {
		fprintf(
			stderr,
			"saveProjectToAsl: failed to add part "
			"(buffer: %s) (file: %s) (len: %d)\n",
			buffer ? buffer : "NULL",
			file   ? file   : "NULL",
			bufferLen);
		goto freeBuffer;
	}

	if(!saveProjectToFile(buffer)) {
		EasyRequest(projectWindow,
			&projectSaveFailEasyStruct,
			NULL,
			buffer);
		goto freeBuffer;
	}
	setProjectFilename(buffer);

	/* TODO: mark things as saved */

freeBuffer:
	free(buffer);
done:
	return;
}

static void saveProjectAs(void) {
	struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
		ASL_Hail, "Save Project As",
		ASL_Window, projectWindow,
		ASL_FuncFlags, FILF_SAVE,
		TAG_END);
	if(!request) {
		goto done;
	}

	if(AslRequest(request, NULL)) {
		saveProjectToAsl(request->rf_Dir, request->rf_File);
	}

	FreeAslRequest(request);
done:
	return;
}

static void saveProject(void) {
	if(*projectFilename) {
		if(!saveProjectToFile(projectFilename)) {
			EasyRequest(projectWindow,
				&projectSaveFailEasyStruct,
				NULL,
				projectFilename);
		}
	} else {
		saveProjectAs();
	}
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
		case 0: newProject(); break;
		case 2: openProject(); break;
		case 4: saveProject(); break;
		case 5: saveProjectAs(); break;
		case 8: selectTilesetPackage(); break;
		case 10: running = 0; break;
	}
}

static void newMap(void) {
	MapEditor *mapEditor = newMapEditorNewMap();
	addToMapEditorList(mapEditor);
	addWindowToSigMask(mapEditor->window);
}

static void handleMapsMenuPick(UWORD itemNum, UWORD subNum) {
	switch(itemNum) {
		case 0: newMap(); break;
	}
}

static void handleMainMenuPick(ULONG menuNumber) {
	UWORD menuNum = MENUNUM(menuNumber);
	UWORD itemNum = ITEMNUM(menuNumber);
	UWORD subNum  = SUBNUM(menuNumber);
	switch(menuNum) {
		case 0: handleProjectMenuPick(itemNum, subNum); break;
		case 1: handleMapsMenuPick(itemNum, subNum); break;
	}
}

static void handleMainMenuPicks(ULONG menuNumber) {
	struct MenuItem *item = NULL;
	while(running && menuNumber != MENUNULL) {
		handleMainMenuPick(menuNumber);
		item = ItemAddress(menu, menuNumber);
		menuNumber = item->NextSelect;
	}
}

static void handleProjectMessage(struct IntuiMessage* msg) {
	switch(msg->Class) {
		case IDCMP_MENUPICK:
			handleMainMenuPicks((ULONG)msg->Code);
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
	if(!tilesetPackage) {
		int choice = EasyRequest(
			mapEditor->window,
			&noTilesetPackageLoadedEasyStruct,
			NULL);
		if(choice) {
			selectTilesetPackage();
		}
	}

	/* even after giving the user the opportunity to set the tileset
	   package, we need to be sure they did so... */
	if(tilesetPackage && !mapEditor->tilesetRequester) {
		TilesetRequester *tilesetRequester = newTilesetRequester();
		if(tilesetRequester) {
			attachTilesetRequesterToMapEditor(mapEditor, tilesetRequester);
			addWindowToSigMask(tilesetRequester->window);
		}
	}
}

static void handleUpdateMapName(MapEditor *mapEditor) {
	struct StringInfo *stringInfo =
		(struct StringInfo*)mapEditor->mapNameGadget->SpecialInfo;
	strcpy(mapEditor->map->name, stringInfo->Buffer);
}

static void handleMapEditorGadgetUp
(MapEditor *mapEditor, struct Gadget *gadget) {
	switch(gadget->GadgetID) {
	case CHOOSE_TILESET_ID:
		handleChooseTilesetClicked(mapEditor);
		break;
	case MAP_NAME_ID:
		handleUpdateMapName(mapEditor);
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

static void saveMapAs(MapEditor *mapEditor) {
    int selected = saveMapRequester(mapEditor);
    if(!selected) {
        return;
    }

    /* TODO: add a prompt if the map is already occupied */

    if(!project.maps[selected-1]) {
        project.mapCnt++;
        project.maps[selected-1] = copyMap(mapEditor->map);
        /* TODO: test for failure in copy */
    } else {
        overwriteMap(mapEditor->map, project.maps[selected-1]);
    }

    mapEditor->mapNum = selected;

    updateProjectMapName(selected - 1, mapEditor->map);
}

static void saveMap(MapEditor *mapEditor) {
	if(!mapEditor->mapNum) {
		saveMapAs(mapEditor);
	} else {
		/* TODO: save map */
	}
}

static void handleMapMenuPick(MapEditor *mapEditor, UWORD itemNum, UWORD subNum) {
	switch(itemNum) {
		case 0: newMap(); break;
		case 4: saveMap(mapEditor); break;
        case 5: saveMapAs(mapEditor); break;
		case 8: mapEditor->closed = 1; break;
	}
}

static void handleMapEditorMenuPick(MapEditor *mapEditor, ULONG menuNumber) {
	UWORD menuNum = MENUNUM(menuNumber);
	UWORD itemNum = ITEMNUM(menuNumber);
	UWORD subNum  = SUBNUM(menuNumber);
	switch(menuNum) {
		case 0: handleMapMenuPick(mapEditor, itemNum, subNum); break;
	}
}

static void handleMapEditorMenuPicks(MapEditor *mapEditor, ULONG menuNumber) {
	struct MenuItem *item = NULL;
	while(!mapEditor->closed && menuNumber != MENUNULL) {
		handleMapEditorMenuPick(mapEditor, menuNumber);
		item = ItemAddress(menu, menuNumber);
		menuNumber = item->NextSelect;
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
	case IDCMP_MOUSEBUTTONS:
		handleMapEditorClick(mapEditor, msg->MouseX, msg->MouseY);
		break;
	case IDCMP_MENUPICK:
		handleMapEditorMenuPick(mapEditor, (ULONG)msg->Code);
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

			if(i->tilesetRequester) {
				removeWindowFromSigMask(i->tilesetRequester->window);
				/* closeMapEditor takes care of everything else */
			}

			removeWindowFromSigMask(i->window);
			closeMapEditor(i);
		}
		i = next;
	}
}

static void handleTilesetRequesterGadgetUp(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
	mapEditorSetTileset(mapEditor, msg->Code);
}

static void handleTilesetRequesterMessage(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
	switch(msg->Class) {
	case IDCMP_CLOSEWINDOW:
		tilesetRequester->closed = 1;
		break;
	case IDCMP_REFRESHWINDOW:
		GT_BeginRefresh(tilesetRequester->window);
		refreshTilesetRequester(tilesetRequester);
		GT_EndRefresh(tilesetRequester->window, TRUE);
		break;
	case IDCMP_GADGETUP:
		handleTilesetRequesterGadgetUp(mapEditor, tilesetRequester, msg);
		break;
	}
}

static void handleTilesetRequesterMessages(MapEditor *mapEditor, TilesetRequester *tilesetRequester) {
	struct IntuiMessage *msg = NULL;
	while(msg = GT_GetIMsg(tilesetRequester->window->UserPort)) {
		handleTilesetRequesterMessage(mapEditor, tilesetRequester, msg);
		GT_ReplyIMsg(msg);
	}
}

static void handleAllTilesetRequesterMessages(long signalSet) {
	MapEditor *i = firstMapEditor;
	while(i) {
		TilesetRequester *tilesetRequester = i->tilesetRequester;
		if(tilesetRequester) {
			if(1L << tilesetRequester->window->UserPort->mp_SigBit & signalSet) {
				handleTilesetRequesterMessages(i, tilesetRequester);
			}
		}
		i = i->next;
	}
}

static void closeDeadTilesetRequesters(void) {
	MapEditor *i = firstMapEditor;
	while(i) {
		if(i->tilesetRequester && i->tilesetRequester->closed) {
			removeWindowFromSigMask(i->tilesetRequester->window);
			closeTilesetRequester(i->tilesetRequester);
			i->tilesetRequester = NULL;
		}
		i = i->next;
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
		handleAllTilesetRequesterMessages(signalSet);
		closeDeadMapEditors();
		closeDeadTilesetRequesters();
	}
}

static void initPalette(struct ViewPort *viewport) {
	LONG i;
	ULONG c = 15;
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

	/* TODO: put these in a list? */
	initMapEditorScreen();
    initMapRequesterScreen();
	initTilesetRequesterScreen();

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

	/* TODO: put these in a list? */
	initMapEditorVi();
    initMapRequesterVi();
	initTilesetRequesterVi();

	menu = CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
	if(!menu) {
		retCode = -5;
		goto freeVisualInfo;
	}

	if(!LayoutMenus(menu, vi, TAG_END)) {
		retCode = -6;
		goto freeMenu;

	}

	if(!initMapEditorMenu()) {
		retCode = -7;
		goto freeMenu;
	}


	SetMenuStrip(projectWindow, menu);

	ActivateWindow(projectWindow);
	
	initProject(&project);

	mainLoop();
	
	retCode = 0;
closeAllMapEditors:
	closeAllMapEditors();
freeTilesetPackage:
	freeTilesetPackage(tilesetPackage);
freeProject:
	freeProject(&project);
clearMenu:
	ClearMenuStrip(projectWindow);
freeMapEditorMenu:
	freeMapEditorMenu();
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
