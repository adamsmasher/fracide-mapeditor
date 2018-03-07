#ifndef FRAMEWORK_WINDOWSET_H
#define FRAMEWORK_WINDOWSET_H

/* TODO: get rid of me */

#include "Window.h"

void addWindowToSet(FrameworkWindow*);
void removeWindowFromSet(FrameworkWindow*);

FrameworkWindow *windowSetFirstWindow(void);

void closeAllWindows(void);

long windowSetSigMask(void);

#endif
