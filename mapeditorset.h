#ifndef FRAC_MAPEDITOR_SET
#define FRAC_MAPEDITOR_SET

#include "MapEditor.h"

void addToMapEditorSet(MapEditor*);
void removeFromMapEditorSet(MapEditor*);

MapEditor *mapEditorSetFirst(void);
MapEditor *findMapEditor(int mapNum);

int ensureMapEditorsSaved(void);

void closeAllMapEditors(void);


#endif
