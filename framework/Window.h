#ifndef FRAMEWORK_WINDOW_H
#define FRAMEWORK_WINDOW_H

#include <intuition/intuition.h>

struct FrameworkWindow_tag;

typedef void (*CloseFunction)(struct FrameworkWindow_tag*);

typedef struct WindowKind_tag {
  struct NewWindow newWindow;
  struct MenuSpec_tag *menuSpec;
  /* TODO: makes me sad that we build a menu for every window... */

  CloseFunction closeWindow;
} WindowKind;

typedef struct FrameworkWindow_tag {
  WindowKind *kind;

  struct Window *intuitionWindow;
  struct Menu   *menu;

  void *data;

  struct FrameworkWindow_tag *children;
  struct FrameworkWindow_tag *next;
} FrameworkWindow;

void handleWindowEvents(FrameworkWindow*, long signalSet);

FrameworkWindow *openWindowOnScreen(WindowKind*, struct Screen*);

void closeWindow(FrameworkWindow*);

#endif
