#include "screen.h"

#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

#include "menubuild.h"
#include "windowset.h"

static struct Screen *screen = NULL;
static void          *vi     = NULL;

BOOL initGlobalScreen(struct NewScreen *newScreen) {
  screen = OpenScreen(newScreen);
  if(!screen) {
    fprintf(stderr, "initGlobalScreen: failed to open screen\n");
    goto error;
  }

  vi = GetVisualInfo(screen, TAG_END);
  if(!vi) {
    fprintf(stderr, "initGlobalScreen: failed to get visual info\n");
    goto error_closeScreen;
  }

  return TRUE;
error_closeScreen:
  CloseScreen(screen);
error:
  return FALSE;
}

void closeGlobalScreen(void) {
  /* TODO: add init checks */
  FreeVisualInfo(vi);
  CloseScreen(screen);
}

FrameworkWindow *openWindowOnGlobalScreen(WindowKind *windowKind) {
  FrameworkWindow *window;

  if(!screen) {
    fprintf(stderr, "openWindowOnScreen: screen not yet initialized\n");
    return NULL;
  }

  /* TODO: move much of the rest into window.c */
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

  return window;
error_closeWindow:
  CloseWindow(window->intuitionWindow);
error_freeWindow:
  free(window);
error:
  return NULL;
}

WORD getScreenWidth(void) {
  if(!screen) {
    fprintf(stderr, "getScreenWidth: screen not yet initialized\n");
    return -1;
  }
  return screen->Width;
}

WORD getScreenHeight(void) {
  if(!screen) {
    fprintf(stderr, "getScreenHeight: screen not yet initialized\n");
    return -1;
  }
  return screen->Height;
}

struct ViewPort *getGlobalViewPort(void) {
  if(!screen) {
    fprintf(stderr, "getGlobalViewPort: screen not yet initialized\n");
    return NULL;
  }
  return &screen->ViewPort;
}

void *getGlobalVi(void) {
  if(!screen) {
    fprintf(stderr, "getGlobalVi: screen not yet initialized\n");
    return NULL;
  }
  return vi;
}
