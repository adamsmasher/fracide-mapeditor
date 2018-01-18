#ifndef FRAMEWORK_WINDOWSET_H
#define FRAMEWORK_WINDOWSET_H

#include "Window.h"

void addWindowToSet(FrameworkWindow*);
void removeWindowFromSet(FrameworkWindow*);

FrameworkWindow *windowSetFirstWindow(void);

/* TODO: I wonder if we don't need to expose this now... */
long windowSetSigMask(void);

#endif
