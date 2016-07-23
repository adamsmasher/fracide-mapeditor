#include <proto/exec.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>

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

	Wait(1 << projectWindow->UserPort->mp_SigBit);

	retCode = 0;
closeWindow:
	CloseWindow(projectWindow);
closeScreen:
	CloseScreen(screen);
closeIntuition:
	CloseLibrary(intuition);
done:
	return retCode;
}
