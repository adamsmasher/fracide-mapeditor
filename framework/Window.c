#include "window.h"

#include <proto/exec.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

/* TODO: get rid of the menu.h in the main program */
#include "framework/menu.h"
#include "menubuild.h"
#include "windowset.h"

FrameworkWindow *openWindowOnScreen(WindowKind *windowKind, struct Screen *screen) {
  FrameworkWindow *window;

  windowKind->newWindow.Screen = screen;

  window = malloc(sizeof(FrameworkWindow));
  if(!window) {
    fprintf(stderr, "openWindowOnGlobalScreen: failed to allocate window\n");
    goto error;
  }

  window->kind = windowKind;
  window->children = NULL;

  window->intuitionWindow = OpenWindow(&windowKind->newWindow);
  if(!window->intuitionWindow) {
    fprintf(stderr, "openWindowOnGlobalScreen: failed to open window\n");
    goto error_freeWindow;
  }

  window->menu = createAndLayoutMenuFromSpec(windowKind->menuSpec);
  if(!window->menu) {
    fprintf(stderr, "openWindowOnGlobalScreen: failed to create menu\n");
    goto error_closeWindow;
  }

  SetMenuStrip(window->intuitionWindow, window->menu);
  addWindowToSet(window);
  GT_RefreshWindow(window->intuitionWindow, NULL);

  return window;
error_closeWindow:
  CloseWindow(window->intuitionWindow);
error_freeWindow:
  free(window);
error:
  return NULL;
}

static void handleWindowChildEvents(FrameworkWindow *window, long signalSet) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    handleWindowEvents(i, signalSet);
    i = next;
  }
}

static void refreshWindow(FrameworkWindow *window) {
  struct Window *iwindow = window->intuitionWindow;
  GT_BeginRefresh(iwindow);
  if(window->kind->refreshWindow) {
    window->kind->refreshWindow(window);
  }
  GT_EndRefresh(iwindow, TRUE);
}

static void dispatchMessage(FrameworkWindow *window, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_MENUPICK:
      invokeMenuHandler(window, msg->Code);
      break;
    case IDCMP_REFRESHWINDOW:
      refreshWindow(window);
      break;
  }
}

void handleWindowEvents(FrameworkWindow *window, long signalSet) {
  struct IntuiMessage *msg;
  struct Window *iwindow = window->intuitionWindow;

  if(1L << iwindow->UserPort->mp_SigBit & signalSet) {
    while(msg = (struct IntuiMessage*)GetMsg(iwindow->UserPort)) {
      dispatchMessage(window, msg);
      ReplyMsg((struct Message*)msg);
    }
  }

  handleWindowChildEvents(window, signalSet);
}

static void closeChildren(FrameworkWindow *window) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    closeWindow(i);
    i = next;
  }
}

void closeWindow(FrameworkWindow *window) {
  closeChildren(window);

  if(window->kind->closeWindow) {
    window->kind->closeWindow(window);
  }

  ClearMenuStrip(window->intuitionWindow);
  FreeMenus(window->menu);

  removeWindowFromSet(window);
  CloseWindow(window->intuitionWindow);

  free(window);
}
