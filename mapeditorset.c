#include "mapeditorset.h"

#include "MapEditor.h"
#include "windowset.h"

static MapEditor *firstMapEditor = NULL;

MapEditor *mapEditorSetFirst(void) {
    return firstMapEditor;
}

void addToMapEditorSet(MapEditor *mapEditor) {
    mapEditor->next = firstMapEditor;
    if(firstMapEditor) {
        firstMapEditor->prev = mapEditor;
    }
    firstMapEditor = mapEditor;
}

void removeFromMapEditorSet(MapEditor *mapEditor) {
    if(mapEditor->next) {
        mapEditor->next->prev = mapEditor->prev;
    }
    if(mapEditor->prev) {
        mapEditor->prev->next = mapEditor->next;
    } else {
        firstMapEditor = mapEditor->next;
    }
}

MapEditor *findMapEditor(int mapNum) {
    MapEditor *mapEditor = firstMapEditor;
    while(mapEditor) {
        if(mapEditor->mapNum - 1 == mapNum) {
            return mapEditor;
        }
        mapEditor = mapEditor->next;
    }
    return NULL;
}

int ensureMapEditorsSaved(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        if(!i->saved && !unsavedMapEditorAlert(i)) {
            return 0;
        }
        i = i->next;
    }
    return 1;
}

void closeAllMapEditors(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        MapEditor *next = i->next;
        removeWindowFromSet(i->window);
        if(i->tilesetRequester) {
            removeWindowFromSet(i->tilesetRequester->window);
        }
        closeMapEditor(i);
        i = next;
    }
    firstMapEditor = NULL;
}
