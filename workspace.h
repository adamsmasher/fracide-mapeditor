#ifndef FRAC_WORKSPACE_H
#define FRAC_WORKSPACE_H

#include "MapEditor.h"

/* TODO: put me...somewhere, too, maybe */
int saveMap(MapEditor*);
int saveMapAs(MapEditor*);

int unsavedMapEditorAlert(MapEditor*);

/* TODO: no. put me in mapeditor set if anywhere. */
extern MapEditor *firstMapEditor;

#endif
