#include "screen.h"

#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

#include "window.h"

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
  if(vi) {
    FreeVisualInfo(vi);
    vi = NULL;
  } else {
    fprintf(stderr, "closeGlobalScreen: vi was NULL\n");
  }

  if(screen) {
    CloseScreen(screen);
    screen = NULL;
  } else {
    fprintf(stderr, "closeGlobalScreen: screen was NULL\n");
  }
}

FrameworkWindow *openWindowOnGlobalScreen(WindowKind *windowKind, WindowGadgets *gadgets) {
  if(!screen) {
    fprintf(stderr, "openWindowOnScreen: screen not yet initialized\n");
    goto error;
  }

  return openWindowOnScreen(windowKind, gadgets, screen);

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
