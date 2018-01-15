#include "screen.h"

#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

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

struct Window *openWindowOnScreen(struct NewWindow *newWindow) {
  if(!screen) {
    fprintf(stderr, "openWindowOnScreen: screen not yet initialized\n");
    return NULL;
  }

  newWindow->Screen = screen;
  return OpenWindow(newWindow);
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
