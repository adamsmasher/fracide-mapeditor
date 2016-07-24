#include "MapEditor.h"

#include <stdlib.h>

MapEditor *newMapEditor(void) {
	MapEditor *mapEditor = malloc(sizeof(MapEditor));
	if(!mapEditor) {
		return NULL;
	}

	mapEditor->prev =   NULL;
	mapEditor->next =   NULL;
	mapEditor->window = NULL;
	return mapEditor;
}
