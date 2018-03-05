#ifndef PROJECT_WINDOW_H
#define PROJECT_WINDOW_H

#include <intuition/intuition.h>

#include "framework/Window.h"
#include "Project.h"
#include "TilesetPackage.h"

#define PROJECT_FILENAME_LENGTH 256

/* TODO: in a different module? */
typedef struct ProjectWindowData_tag {
  Project project;
  BOOL projectSaved;
  char projectFilename[PROJECT_FILENAME_LENGTH];
  TilesetPackage *tilesetPackage;
} ProjectWindowData;

FrameworkWindow *getProjectWindow(void);

BOOL openProjectWindow(void);
void closeProjectWindow(void);

void newProject(FrameworkWindow*);
BOOL saveProject(FrameworkWindow*);
BOOL saveProjectAs(FrameworkWindow*);
void openProject(FrameworkWindow*);
void revertProject(FrameworkWindow*);
void selectTilesetPackage(FrameworkWindow*);
void quit(FrameworkWindow*);

#endif
