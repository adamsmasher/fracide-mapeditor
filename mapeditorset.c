#include "mapeditorset.h"

#include "MapEditor.h"
#include "workspace.h"

MapEditor *firstMapEditor = NULL;

MapEditor *mapEditorSetFirst(void) {
    return firstMapEditor;
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
