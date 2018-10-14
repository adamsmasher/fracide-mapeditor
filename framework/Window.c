#include "window.h"

#include <proto/exec.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>

#include "GadgetEvents.h"
#include "menu.h"
#include "menubuild.h"

FrameworkWindow *openWindowOnScreen(WindowKind *windowKind, struct Screen *screen, void *data) {
  FrameworkWindow *window;

  windowKind->newWindow.Screen = screen;

  window = malloc(sizeof(FrameworkWindow));
  if(!window) {
    fprintf(stderr, "openWindowOnScreen: failed to allocate window\n");
    goto error;
  }

  window->data = data;
  window->kind = windowKind;
  window->parent = NULL;
  window->children = NULL;
  window->next = NULL;
  window->prev = NULL;
  window->closed = FALSE;

  if(windowKind->buildGadgets) {
    window->gadgets = (*windowKind->buildGadgets)(windowKind->newWindow.Width, windowKind->newWindow.Height, data);
    if(!window->gadgets) {
      fprintf(stderr, "openWindowOnScreen: failed to build gadgets\n");
      goto error_freeWindow;
    }
    windowKind->newWindow.FirstGadget = window->gadgets->glist;
  } else {
    window->gadgets = NULL;
  }

  window->intuitionWindow = OpenWindow(&windowKind->newWindow);
  if(!window->intuitionWindow) {
    fprintf(stderr, "openWindowOnScreen: failed to open window\n");
    goto error_freeGadgets;
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
error_freeGadgets:
  if(windowKind->freeGadgets) {
    (*windowKind->freeGadgets)(window->gadgets);
  }
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

FrameworkWindow *openChildWindow(FrameworkWindow *parent, WindowKind *windowKind, void *data) {
  FrameworkWindow *child = openWindowOnScreen(windowKind, parent->intuitionWindow->WScreen, data);
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

static void resizeWindow(FrameworkWindow *window) {
  if(window->gadgets) {
    RemoveGList(window->intuitionWindow, window->gadgets->glist, -1);
    (*window->kind->freeGadgets)(window->gadgets);
    SetRast(window->intuitionWindow->RPort, 0);
    window->gadgets = (*window->kind->buildGadgets)(window->intuitionWindow->Width, window->intuitionWindow->Height, window->data);
    if(!window->gadgets) {
      fprintf(stderr, "resizeWindow: couldn't create gadgets\n");
      goto error;
    }
    AddGList(window->intuitionWindow, window->gadgets->glist, (UWORD)~0, -1, NULL);
    RefreshWindowFrame(window->intuitionWindow);
    RefreshGList(window->gadgets->glist, window->intuitionWindow, NULL, -1);
    GT_RefreshWindow(window->intuitionWindow, NULL);
  }

  return;

error:
  return;
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
    case IDCMP_NEWSIZE:
      resizeWindow(window);
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

  if(window->kind->freeGadgets) {
    (*window->kind->freeGadgets)(window->gadgets);
  }

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
