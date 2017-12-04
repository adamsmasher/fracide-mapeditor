#ifndef FRAC_WORKSPACE_H
#define FRAC_WORKSPACE_H

#include "MapEditor.h"

void newProject(void);
void openProject(void);
int saveProject(void);
int saveProjectAs(void);
void revertProject(void);
void selectTilesetPackage(void);
void quit(void);

void showEntityEditor(void);
void refreshAllEntityBrowsers(void);

void showSongNamesEditor(void);
void refreshAllSongDisplays(void);

void newMap(void);
void openMap(void);
int openMapNum(int);
void closeAllMapEditors(void);

/* TODO: no. */
extern MapEditor *firstMapEditor;

#endif
