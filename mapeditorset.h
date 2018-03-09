#ifndef FRAC_MAPEDITOR_SET
#define FRAC_MAPEDITOR_SET

#include "MapEditor.h"

void addToMapEditorSet(MapEditor*);
void removeFromMapEditorSet(MapEditor*);

int ensureMapEditorsSaved(void);

void closeAllMapEditors(void);


#endif
