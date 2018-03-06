#ifndef FRAC_WORKSPACE_H
#define FRAC_WORKSPACE_H

#include "MapEditor.h"

void refreshAllEntityBrowsers(void);

void refreshAllSongDisplays(void);

/* TODO: put me...somewhere, too, maybe */
void newMap(void);
int saveMap(MapEditor*);
int saveMapAs(MapEditor*);

int unsavedMapEditorAlert(MapEditor*);

/* TODO: no. put me in mapeditor set if anywhere. */
extern MapEditor *firstMapEditor;

#endif
