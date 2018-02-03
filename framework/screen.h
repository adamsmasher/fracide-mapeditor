#ifndef FRAMEWORK_SCREEN_H
#define FRAMEWORK_SCREEN_H

#include <intuition/intuition.h>

#include "Window.h"

BOOL initGlobalScreen(struct NewScreen*);
void closeGlobalScreen(void);

WORD getScreenWidth(void);
WORD getScreenHeight(void);

FrameworkWindow *openWindowOnGlobalScreen(WindowKind*);

struct ViewPort *getGlobalViewPort(void);
void            *getGlobalVi(void);

#endif
