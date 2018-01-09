#ifndef FRAC_WORKSPACE_H
#define FRAC_WORKSPACE_H

#include "MapEditor.h"

/* TODO: move all these project things into currentproject.h */
void newProject(void);
void openProject(void);
void revertProject(void);
void selectTilesetPackage(void);
void quit(void);

void showEntityEditor(void);
void refreshAllEntityBrowsers(void);

void refreshAllSongDisplays(void);

/* TODO: put me...somewhere, too, maybe */
void newMap(void);
void openMap(void);
int openMapNum(int);
int saveMap(MapEditor*);
int saveMapAs(MapEditor*);

int unsavedMapEditorAlert(MapEditor*);

/* TODO: no. put me in mapeditor set if anywhere. */
extern MapEditor *firstMapEditor;

#endif
