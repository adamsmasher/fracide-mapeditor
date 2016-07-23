#include <proto/exec.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>

static struct Library *intuition = NULL;

static struct Screen *screen = NULL;
static struct NewScreen newScreen = {
	0,0,640,512,2,
	0,1,
	HIRES|LACE,
	CUSTOMSCREEN,
	NULL,
	"FracIDE Map Editor",
	NULL,
	NULL
};	

static struct Window *myWindow = NULL;
static struct NewWindow myNewWindow = {
	0,12,
	200,100,
	0xFF,0xFF,
	CLOSEWINDOW,
	WINDOWCLOSE | WINDOWDEPTH | WINDOWDRAG | WINDOWSIZING | ACTIVATE,
	NULL,
	NULL,
	"Close Me",
	NULL,
	NULL,
	80,24,
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

	myNewWindow.Screen = screen;
	myWindow = OpenWindow(&myNewWindow);
	if(!myWindow) {
		retCode = -3;
		goto closeScreen;
	}	

	Wait(1 << myWindow->UserPort->mp_SigBit);

	retCode = 0;
closeWindow:
	CloseWindow(myWindow);
closeScreen:
	CloseScreen(screen);
closeIntuition:
	CloseLibrary(intuition);
done:
	return retCode;
}
