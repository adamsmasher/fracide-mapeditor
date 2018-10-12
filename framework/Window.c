#include "window.h"

#include <proto/exec.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

#include "GadgetEvents.h"
#include "menu.h"
#include "menubuild.h"

FrameworkWindow *openWindowOnScreen(WindowKind *windowKind, WindowGadgets *gadgets, struct Screen *screen) {
  FrameworkWindow *window;

  windowKind->newWindow.Screen = screen;

  window = malloc(sizeof(FrameworkWindow));
  if(!window) {
    fprintf(stderr, "openWindowOnScreen: failed to allocate window\n");
    goto error;
  }

  window->gadgets = gadgets;
  if(gadgets) {
    windowKind->newWindow.FirstGadget = gadgets->glist;
  }

  window->kind = windowKind;
  window->parent = NULL;
  window->children = NULL;
  window->next = NULL;
  window->prev = NULL;
  window->closed = FALSE;

  window->intuitionWindow = OpenWindow(&windowKind->newWindow);
  if(!window->intuitionWindow) {
    fprintf(stderr, "openWindowOnScreen: failed to open window\n");
    goto error_freeWindow;
  }

  window->treeSigMask = 1L << window->intuitionWindow->UserPort->mp_SigBit;

  if(windowKind->menuSpec) {
    window->menu = createAndLayoutMenuFromSpec(windowKind->menuSpec);
    if(!window->menu) {
      fprintf(stderr, "openWindowOnScreen: failed to create menu\n");
      goto error_closeWindow;
    }
    SetMenuStrip(window->intuitionWindow, window->menu);
  } else {
    window->menu = NULL;
  }

  GT_RefreshWindow(window->intuitionWindow, NULL);

  /* redraw the whole window */
  if(window->kind->refreshWindow) {
    (*window->kind->refreshWindow)(window);
  }

  return window;
error_closeWindow:
  CloseWindow(window->intuitionWindow);
error_freeWindow:
  free(window);
error:
  return NULL;
}

static void propagateSigMaskUp(FrameworkWindow *child) {
  FrameworkWindow *parent = child->parent;
  if(parent) {
    parent->treeSigMask |= child->treeSigMask;
    propagateSigMaskUp(parent);
  }
}

static void attachChildWindow(FrameworkWindow *parent, FrameworkWindow *child) {
  child->parent = parent;
  child->prev   = NULL;
  child->next   = parent->children;

  parent->children = child;

  propagateSigMaskUp(child);
}

FrameworkWindow *openChildWindow(FrameworkWindow *parent, WindowKind *windowKind, WindowGadgets *gadgets) {
  FrameworkWindow *child = openWindowOnScreen(windowKind, gadgets, parent->intuitionWindow->WScreen);
  attachChildWindow(parent, child);
  return child;
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
    (*window->kind->refreshWindow)(window);
  }
  GT_EndRefresh(iwindow, TRUE);
}

static BOOL canCloseWindow(FrameworkWindow*);

static BOOL canCloseChildren(FrameworkWindow *window) {
  FrameworkWindow *i = window->children;
  while(i) {
    if(!canCloseWindow(i)) {
      return FALSE;
    }
    i = i->next;
  }
  return TRUE;
}

static BOOL canCloseWindow(FrameworkWindow *window) {
  if(!canCloseChildren(window)) {
    return FALSE;
  }

  if(window->kind->canCloseWindow) {
    return (*window->kind->canCloseWindow)(window);
  } else {
    return TRUE;
  }
}

BOOL tryToCloseWindow(FrameworkWindow *window) {
  if(canCloseWindow(window)) {
    window->closed = TRUE;
    return TRUE;
  } else {
    return FALSE;
  }
}

static void invokeGadgetUpHandler(FrameworkWindow *window, struct Gadget *gadget, UWORD code) {
  GadgetUpHandler handler = findHandlerForGadgetUp(gadget);
  if(handler) {
    (*handler)(window, code);
  }
}

static void invokeClickHandler(FrameworkWindow *window, WORD x, WORD y) {
  if(window->kind->clickOnWindow) {
    (*window->kind->clickOnWindow)(window, x, y);
  }
}

static void dispatchMessage(FrameworkWindow *window, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_MENUPICK:
      invokeMenuHandler(window, (ULONG)msg->Code);
      break;
    case IDCMP_REFRESHWINDOW:
      refreshWindow(window);
      break;
    case IDCMP_CLOSEWINDOW:
      tryToCloseWindow(window);
      break;
    case IDCMP_GADGETUP:
      invokeGadgetUpHandler(window, (struct Gadget*)msg->IAddress, msg->Code);
      break;
    case IDCMP_MOUSEBUTTONS:
      invokeClickHandler(window, msg->MouseX, msg->MouseY);
      break;
  }
}

void handleWindowEvents(FrameworkWindow *window, long signalSet) {
  struct IntuiMessage *msg;
  struct Window *iwindow = window->intuitionWindow;

  if(1L << iwindow->UserPort->mp_SigBit & signalSet) {
    while(msg = GT_GetIMsg(iwindow->UserPort)) {
      dispatchMessage(window, msg);
      GT_ReplyIMsg(msg);
    }
  }

  handleWindowChildEvents(window, signalSet);
}

static void forceCloseChildren(FrameworkWindow *window) {
  FrameworkWindow *i = window->children;
  while(i) {
    FrameworkWindow *next = i->next;
    forceCloseWindow(i);
    i = next;
  }
}

static void removeChildWindow(FrameworkWindow *parent, FrameworkWindow *child) {
  if(parent->children == child) {
    parent->children = child->next;
  }
  parent->treeSigMask &= ~child->treeSigMask;
}

void forceCloseWindow(FrameworkWindow *window) {
  forceCloseChildren(window);

  if(window->menu) {
    ClearMenuStrip(window->intuitionWindow);
    FreeMenus(window->menu);
  }

  CloseWindow(window->intuitionWindow);

  if(window->kind->closeWindow) {
    (*window->kind->closeWindow)(window);
  }

  if(window->parent) {
    removeChildWindow(window->parent, window);
  }

  if(window->next) {
    window->next->prev = window->prev;
  }

  if(window->prev) {
    window->prev->next = window->next;
  }

  free(window);
}
