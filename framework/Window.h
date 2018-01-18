#ifndef FRAMEWORK_WINDOW_H
#define FRAMEWORK_WINDOW_H

#include <intuition/intuition.h>

struct FrameworkWindow_tag;

typedef void (*EventHandler)(struct FrameworkWindow_tag*, long signalSet);

typedef struct WindowKind_tag {
  EventHandler handleEvents;
} WindowKind;

typedef struct FrameworkWindow_tag {
  WindowKind *kind;
  struct Window *intuitionWindow;
  struct FrameworkWindow_tag *next;
} FrameworkWindow;

#endif
