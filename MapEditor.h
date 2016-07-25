#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

typedef struct MapEditorTag {
	struct MapEditorTag *next;
	struct MapEditorTag *prev;
	struct Window *window;
	int closed;
} MapEditor;

void initMapEditorScreen(void);

MapEditor *newMapEditor(void);
void closeMapEditor(MapEditor*);

#endif
