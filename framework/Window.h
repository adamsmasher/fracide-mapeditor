#ifndef FRAMEWORK_WINDOW_H
#define FRAMEWORK_WINDOW_H

#include <intuition/intuition.h>

typedef void (*RefreshFunction) (struct FrameworkWindow_tag*);
typedef BOOL (*CanCloseFunction)(struct FrameworkWindow_tag*);
typedef void (*CloseFunction)   (struct FrameworkWindow_tag*);

typedef struct WindowKind_tag {
  struct NewWindow newWindow;
  struct MenuSpec_tag *menuSpec;
  /* TODO: makes me sad that we build a menu for every window... */

  RefreshFunction  refreshWindow;
  CanCloseFunction canCloseWindow;
  CloseFunction    closeWindow;
} WindowKind;

typedef struct FrameworkWindow_tag {
  WindowKind *kind;

  struct Window *intuitionWindow;
  struct Menu   *menu;

  void *data;

  BOOL closed;

  struct FrameworkWindow_tag *parent;
  struct FrameworkWindow_tag *children;
  /* These are managed by the windowset */
  struct FrameworkWindow_tag *next;
  struct FrameworkWindow_tag *prev;
} FrameworkWindow;

void handleWindowEvents(FrameworkWindow*, long signalSet);

FrameworkWindow *openWindowOnScreen(WindowKind*, struct Screen*);

void closeWindow(FrameworkWindow*);

#endif
