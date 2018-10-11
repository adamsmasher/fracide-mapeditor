#ifndef FRAMEWORK_WINDOW_H
#define FRAMEWORK_WINDOW_H

#include <intuition/intuition.h>

#include "WindowGadgets.h"

typedef void (*RefreshFunction) (struct FrameworkWindow_tag*);
typedef BOOL (*CanCloseFunction)(struct FrameworkWindow_tag*);
typedef void (*CloseFunction)   (struct FrameworkWindow_tag*);
typedef void (*ClickFunction)   (struct FrameworkWindow_tag*, WORD x, WORD y);

/* TODO: provide a function to build the gadgets..., then auto-rebuild on resize */
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

  WindowGadgets *gadgets;

  void *data;

  BOOL closed;

  long treeSigMask;
  struct FrameworkWindow_tag *parent;
  struct FrameworkWindow_tag *children;
  struct FrameworkWindow_tag *next;
  struct FrameworkWindow_tag *prev;
} FrameworkWindow;

void handleWindowEvents(FrameworkWindow*, long signalSet);

FrameworkWindow *openWindowOnScreen(WindowKind*, WindowGadgets*, struct Screen*);
FrameworkWindow *openChildWindow(FrameworkWindow *parent, WindowKind*, WindowGadgets*);

BOOL tryToCloseWindow(FrameworkWindow*);
void forceCloseWindow(FrameworkWindow*);

#endif
