#include "ProjectWindow.h"

#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>

#include <stdio.h>

#include "globals.h"
#include "menu.h"
#include "windowset.h"

static struct Window *projectWindow = NULL;
static struct Menu   *menu          = NULL;

static struct NewWindow projectNewWindow = {
    0,0, -1, -1,
    0xFF,0xFF,
    MENUPICK,
    BORDERLESS|BACKDROP,
    NULL,
    NULL,
    "Project",
    NULL,
    NULL,
    -1, -1,
    0xFFFF,0xFFFF,
    CUSTOMSCREEN
};

struct Window *getProjectWindow(void) {
    return projectWindow;
}

BOOL openProjectWindow(struct Screen *screen) {
    if(projectWindow) {
        fprintf(stderr, "openProjectWindow: cannot be called when project window already exists\n");
        goto error;
    }

    projectNewWindow.Screen    = screen;
    projectNewWindow.MinWidth  = projectNewWindow.Width  = screen->Width;
    projectNewWindow.MinHeight = projectNewWindow.Height = screen->Height;
    projectWindow = OpenWindow(&projectNewWindow);
    if(!projectWindow) {
        fprintf(stderr, "openProjectWindow: failed to open window!\n");
        goto error;
    }

    menu = createMainMenu();
    if(!menu) {
        fprintf(stderr, "openProjectWindow: Error creating menu\n");
        goto error_freeWindow;
    }

    if(!LayoutMenus(menu, vi, TAG_END)) {
        fprintf(stderr, "openProjectWindow: Error laying out menu\n");
        goto error_freeMenu;
    }

    SetMenuStrip(projectWindow, menu);

    addWindowToSet(projectWindow);

    ActivateWindow(projectWindow);

    return TRUE;

error_freeMenu:
    FreeMenus(menu);
    menu = NULL;
error_freeWindow:
    CloseWindow(projectWindow);
    projectWindow = NULL;
error:
    return FALSE;
}

void closeProjectWindow(void) {
    if(!projectWindow) {
        fprintf(stderr, "closeProjectWindow: projectWindow not yet opened!\n");
        return;
    }

    ClearMenuStrip(projectWindow);
    FreeMenus(menu);
    menu = NULL;

    removeWindowFromSet(projectWindow);
    CloseWindow(projectWindow);

    projectWindow = NULL;
}

static void handleProjectMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
        case IDCMP_MENUPICK:
            handleMainMenuPick(menu, msg);
    }
}

void handleProjectMessages(long signalSet) {
    struct IntuiMessage *msg;

    if(!projectWindow) {
        fprintf(stderr, "handleProjectMessages: called before projectWindow was created\n");
        return;
    }

    if(1L << projectWindow->UserPort->mp_SigBit & signalSet) {
        while(msg = (struct IntuiMessage*)GetMsg(projectWindow->UserPort)) {
            handleProjectMessage(msg);
            ReplyMsg((struct Message*)msg);
        }
    }
}
