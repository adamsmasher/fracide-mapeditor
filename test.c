#include <proto/exec.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#define SCR_WIDTH  640
#define SCR_HEIGHT 512

static struct Library *intuition = NULL;

static struct Screen *screen = NULL;
static struct NewScreen newScreen = {
	0,0,SCR_WIDTH,SCR_HEIGHT,2,
	0,1,
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

static APTR vi = NULL;

static struct NewMenu newMenu[] = {
	{ NM_TITLE, "Project", 0, 0, 0, 0 },
		{ NM_ITEM, "Select Tileset...", 0,   0, 0, 0 },
		{ NM_ITEM, NM_BARLABEL,         0,   0, 0, 0 },
		{ NM_ITEM, "Quit",              "Q", 0, 0, 0 },
	{ NM_END,   NULL,      0, 0, 0, 0 }
};
static struct Menu *menu = NULL;

static int running = 0;

static void handleProjectMenuPick(UWORD itemNum, UWORD subNum) {
	switch(itemNum) {
		case 2: running = 0;
	}
}

static void handleMenuPick(UWORD menuNumber) {
	UWORD menuNum = MENUNUM(menuNumber);
	UWORD itemNum = ITEMNUM(menuNumber);
	UWORD subNum  = SUBNUM(menuNumber);
	switch(menuNum) {
		case 0: handleProjectMenuPick(itemNum, subNum);
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

static void handleMessage(struct IntuiMessage* msg) {
	switch(msg->Class) {
		case IDCMP_MENUPICK:
			handleMenuPick(msg->Code);
	}
}

static void mainLoop(void) {
	struct IntuiMessage *msg = NULL;
	running = 1;
	while(running) {
		Wait(1L << projectWindow->UserPort->mp_SigBit);
		msg = (struct IntuiMessage*)GetMsg(projectWindow->UserPort);
		if(msg) {
			handleMessage(msg);
			ReplyMsg((struct Message*)msg);
		}
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

	projectNewWindow.Screen = screen;
	projectWindow = OpenWindow(&projectNewWindow);
	if(!projectWindow) {
		retCode = -3;
		goto closeScreen;
	}

	vi = GetVisualInfo(screen, TAG_END);
	if(!vi) {
		retCode = -4;
		goto closeWindow;
	}

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
clearMenu:
	ClearMenuStrip(projectWindow);
freeMenu:
	FreeMenus(menu);
freeVisualInfo:
	FreeVisualInfo(vi);
closeWindow:
	CloseWindow(projectWindow);
closeScreen:
	CloseScreen(screen);
closeIntuition:
	CloseLibrary(intuition);
done:
	return retCode;
}
