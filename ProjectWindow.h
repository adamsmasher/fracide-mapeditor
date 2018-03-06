#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"

/* TODO: get rid of me if you can */
FrameworkWindow *getProjectWindow(void);

/* TODO: maybe also us */
BOOL openProjectWindow(void);
void closeProjectWindow(void);

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
