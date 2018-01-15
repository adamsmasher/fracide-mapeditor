#ifndef FRAMEWORK_SCREEN_H
#define FRAMEWORK_SCREEN_H

#include <intuition/intuition.h>

BOOL initGlobalScreen(struct NewScreen*);
void closeGlobalScreen(void);

WORD getScreenWidth(void);
WORD getScreenHeight(void);

/* TODO: call me OnGlobalScreen? */
struct Window *openWindowOnScreen(struct NewWindow*);

struct ViewPort *getGlobalViewPort(void);
void            *getGlobalVi(void);

#endif
