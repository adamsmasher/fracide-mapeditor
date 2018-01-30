#include "ProjectWindow.h"

#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>

#include <stdio.h>

#include "framework/screen.h"
#include "framework/windowset.h"

#include "globals.h"
#include "menu.h"

static FrameworkWindow *projectWindow = NULL;
static struct Menu     *menu          = NULL;

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

static void handleProjectMessage(struct IntuiMessage* msg) {
  switch(msg->Class) {
    case IDCMP_MENUPICK:
      handleMainMenuPick(menu, msg);
  }
}

static void handleProjectMessages(FrameworkWindow *window, long signalSet) {
  struct IntuiMessage *msg;
  struct Window *iwindow = window->intuitionWindow;

  if(1L << iwindow->UserPort->mp_SigBit & signalSet) {
    while(msg = (struct IntuiMessage*)GetMsg(iwindow->UserPort)) {
      handleProjectMessage(msg);
      ReplyMsg((struct Message*)msg);
    }
  }
}

/* TODO: fix me */
static WindowKind projectWindowKind = {
  handleProjectMessages,
  NULL
};

FrameworkWindow *getProjectWindow(void) {
  return projectWindow;
}

BOOL openProjectWindow(void) {
  /* TODO: plan of attack

  project window is a framework window rather than a window

  create a means to attach a menu to a framework window */
  if(projectWindow) {
    fprintf(stderr, "openProjectWindow: cannot be called when project window already exists\n");
    goto error;
  }

  projectNewWindow.MinWidth  = projectNewWindow.Width  = getScreenWidth();
  projectNewWindow.MinHeight = projectNewWindow.Height = getScreenHeight();
  projectWindow = openWindowOnGlobalScreen(&projectWindowKind, &projectNewWindow);
  if(!projectWindow) {
    fprintf(stderr, "openProjectWindow: failed to open window!\n");
    goto error;
  }

  menu = createMainMenu();
  if(!menu) {
    fprintf(stderr, "openProjectWindow: Error creating menu\n");
    goto error_freeWindow;
  }

  SetMenuStrip(projectWindow, menu);

  ActivateWindow(projectWindow->intuitionWindow);

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

    /* TODO: fix me */
    /* removeWindowFromSet(projectWindow); */
    CloseWindow(projectWindow);

    projectWindow = NULL;
}
