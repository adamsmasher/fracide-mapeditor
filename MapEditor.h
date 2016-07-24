#ifndef MAP_EDITOR_H
#define MAP_EDITOR_H

typedef struct MapEditorTag {
	struct MapEditorTag *next;
	struct MapEditorTag *prev;
	struct Window *window;
} MapEditor;

MapEditor *newMapEditor(void);

#endif
