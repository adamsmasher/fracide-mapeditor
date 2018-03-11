#include "workspace.h"

#include <libraries/asl.h>
#include <proto/asl.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "easystructs.h"
#include "globals.h"
#include "project.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "ProjectWindow.h"

int saveMapAs(MapEditor *mapEditor) {
    int selected = saveMapRequester(mapEditor);
    if(!selected) {
        return 0;
    }

    if(1/* TODO: fix me: !currentProjectHasMap(selected - 1) */) {
        if(0/* TODO: fix me: !currentProjectSaveNewMap(mapEditor->map, selected - 1) */) {
            fprintf(stderr, "saveMapAs: failed to save map\n");
            return 0;
        }
    } else {
        int response = EasyRequest(
            mapEditor->window->intuitionWindow,
            &saveIntoFullSlotEasyStruct,
            NULL,
            selected - 1, NULL /* FIXME currentProjectGetMapName(selected - 1) */);
        if(response) {
            /* TODO: fix me */
            /*currentProjectOverwriteMap(mapEditor->map, selected - 1);*/
        } else {
            return 0;
        }
    }

    mapEditorSetMapNum(mapEditor, selected - 1);
    enableMapRevert(mapEditor);

    mapEditorSetSaveStatus(mapEditor, SAVED);

    /* TODO: fix me */
    /* updateCurrentProjectMapName(selected - 1, mapEditor->map); */

    return 1;
}

int saveMap(MapEditor *mapEditor) {
    if(!mapEditor->mapNum) {
        return saveMapAs(mapEditor);
    } else {
        /* TODO: fix me */
        /* currentProjectOverwriteMap(mapEditor->map, mapEditor->mapNum - 1); */
        /* TODO: this is what sets the saved status, but that feels fragile */
        /* TODO: fix me */
        /* updateCurrentProjectMapName(mapEditor->mapNum - 1, mapEditor->map);*/
        mapEditorSetSaveStatus(mapEditor, SAVED);
        return 1;
    }
}
