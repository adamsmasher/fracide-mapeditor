#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"

FrameworkWindow *getProjectWindow(void);

BOOL openProjectWindow(void);
void closeProjectWindow(void);

#endif
