#ifndef FRAMEWORK_WINDOW_H
#define FRAMEWORK_WINDOW_H

#include <intuition/intuition.h>

typedef void (*RefreshFunction) (struct FrameworkWindow_tag*);
typedef BOOL (*CanCloseFunction)(struct FrameworkWindow_tag*);
typedef void (*CloseFunction)   (struct FrameworkWindow_tag*);
typedef void (*ClickFunction)   (struct FrameworkWindow_tag*, WORD x, WORD y);

typedef struct WindowKind_tag {
  struct NewWindow newWindow;
  struct MenuSpec_tag *menuSpec;
  /* TODO: makes me sad that we build a menu for every window... */

  RefreshFunction  refreshWindow;
  CanCloseFunction canCloseWindow;
  CloseFunction    closeWindow;
  ClickFunction    clickOnWindow;
} WindowKind;

typedef struct FrameworkWindow_tag {
  WindowKind *kind;

  struct Window *intuitionWindow;
  struct Menu   *menu;
  struct Gadget *gadgets;

  void *data;

  BOOL closed;

  long treeSigMask;
  struct FrameworkWindow_tag *parent;
  struct FrameworkWindow_tag *children;
  struct FrameworkWindow_tag *next;
  struct FrameworkWindow_tag *prev;
} FrameworkWindow;

void handleWindowEvents(FrameworkWindow*, long signalSet);

/* n.b.: this window takes ownership of its gadgets and frees them on close */
FrameworkWindow *openWindowOnScreen(WindowKind*, struct Gadget*, struct Screen*);

BOOL tryToCloseWindow(FrameworkWindow*);
void forceCloseWindow(FrameworkWindow*);

#endif
