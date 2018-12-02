#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"

FrameworkWindow *newProjectWindow(void);

void projectWindowNewProject(FrameworkWindow*);
BOOL projectWindowSaveProject(FrameworkWindow*);
BOOL projectWindowSaveProjectAs(FrameworkWindow*);
void projectWindowOpenProject(FrameworkWindow*);
void projectWindowRevertProject(FrameworkWindow*);
void projectWindowSelectTilesetPackage(FrameworkWindow*);
void projectWindowQuit(FrameworkWindow*);

void projectWindowNewMap(FrameworkWindow*);
void projectWindowOpenMap(FrameworkWindow*);
BOOL projectWindowOpenMapNum(FrameworkWindow*, int mapNum);

void projectWindowShowEntityNamesEditor(FrameworkWindow*);
void projectWindowShowSongNamesEditor(FrameworkWindow*);

void projectWindowRefreshAllEntityBrowsers(FrameworkWindow*);
void projectWindowRefreshAllSongDisplays(FrameworkWindow*);
void projectWindowRefreshAllEntityBrowsers(FrameworkWindow*);

#endif
