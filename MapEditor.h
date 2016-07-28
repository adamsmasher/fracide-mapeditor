#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

#include <intuition/intuition.h>

#define CURRENT_TILESET_ID (0)
#define CHOOSE_TILESET_ID  (CURRENT_TILESET_ID + 1)

typedef struct MapEditorTag {
	struct MapEditorTag *next;
	struct MapEditorTag *prev;
	struct Window *window;
	struct Gadget *gadgets;
	int closed;
} MapEditor;

void initMapEditorScreen(void);
void initMapEditorVi(void);

MapEditor *newMapEditor(void);
void closeMapEditor(MapEditor*);

#endif
