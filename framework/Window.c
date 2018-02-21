#include "window.h"

#include <proto/exec.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdlib.h>

/* TODO: get rid of the menu.h in the main program */
#include "framework/menu.h"
#include "windowset.h"

static void handleWindowChildEvents(FrameworkWindow *window, long signalSet) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    handleWindowEvents(i, signalSet);
    i = next;
  }
}

static void dispatchMessage(FrameworkWindow *window, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_MENUPICK:
      invokeMenuHandler(window, msg->Code);
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
  window->kind->closeWindow(window);

  ClearMenuStrip(window->intuitionWindow);
  FreeMenus(window->menu);

  removeWindowFromSet(window);
  CloseWindow(window->intuitionWindow);

  free(window);
}
