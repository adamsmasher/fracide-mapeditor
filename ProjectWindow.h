#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"

FrameworkWindow *openProjectWindow(void);

void newProject(FrameworkWindow*);
BOOL saveProject(FrameworkWindow*);
BOOL saveProjectAs(FrameworkWindow*);
void openProject(FrameworkWindow*);
void revertProject(FrameworkWindow*);
void selectTilesetPackage(FrameworkWindow*);
void quit(FrameworkWindow*);

void openMap(FrameworkWindow*);
BOOL openMapNum(FrameworkWindow*, int mapNum);

#endif
