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

static WindowKind projectWindowKind = {
  {
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
  },
  (MenuSpec*)        NULL, /* set me later */
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL
};

FrameworkWindow *getProjectWindow(void) {
  return projectWindow;
}

static void makeWindowFullScreen(void) {
  struct NewWindow *newWindow = &projectWindowKind.newWindow;
  newWindow->MinWidth  = newWindow->Width  = getScreenWidth();
  newWindow->MinHeight = newWindow->Height = getScreenHeight();
}

BOOL openProjectWindow(void) {
  if(projectWindow) {
    fprintf(stderr, "openProjectWindow: cannot be called when project window already exists\n");
    goto error;
  }

  projectWindowKind.menuSpec = mainMenuSpec;

  makeWindowFullScreen();

  projectWindow = openWindowOnGlobalScreen(&projectWindowKind);
  if(!projectWindow) {
    fprintf(stderr, "openProjectWindow: failed to open window!\n");
    goto error;
  }

  ActivateWindow(projectWindow->intuitionWindow);

  return TRUE;

error:
  return FALSE;
}

void closeProjectWindow(void) {
  if(!projectWindow) {
    fprintf(stderr, "closeProjectWindow: projectWindow not yet opened!\n");
    return;
  }

  closeWindow(projectWindow);

  projectWindow = NULL;
}
