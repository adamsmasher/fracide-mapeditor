#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"

FrameworkWindow *openProjectWindow(void);

void newProject(FrameworkWindow *projectWindow);
BOOL saveProject(FrameworkWindow *projectWindow);
BOOL saveProjectAs(FrameworkWindow *projectWindow);
void openProject(FrameworkWindow *projectWindow);
void revertProject(FrameworkWindow *projectWindow);
void selectTilesetPackage(FrameworkWindow *projectWindow);
void quit(FrameworkWindow *projectWindow);

void openMap(FrameworkWindow *projectWindow);
BOOL openMapNum(FrameworkWindow *projectWindow, int mapNum);

void refreshAllSongDisplays(FrameworkWindow *projectWindow);
void refreshAllEntityBrowsers(FrameworkWindow *projectWindow);

#endif
