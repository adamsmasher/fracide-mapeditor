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

#include "framework/runstate.h"
#include "framework/windowset.h"

#include "easystructs.h"
#include "globals.h"
#include "project.h"
#include "MapEditor.h"
#include "mapeditorset.h"
#include "MapRequester.h"
#include "ProjectWindow.h"

void refreshAllSongDisplays(void) {
    MapEditor *i = mapEditorSetFirst();
    while(i) {
        if(i->songRequester) {
            GT_RefreshWindow(i->songRequester->window->intuitionWindow, NULL);
        }
        mapEditorRefreshSong(i);
        i = i->next;
    }
}

void refreshAllEntityBrowsers(void) {
    MapEditor *i = mapEditorSetFirst();
    while(i) {
        if(i->entityBrowser) {
            GT_RefreshWindow(i->entityBrowser->window->intuitionWindow, NULL);
        }
        i = i->next;
    }
}

void newMap(void) {
    MapEditor *mapEditor = newMapEditorNewMap();
    if(!mapEditor) {
        fprintf(stderr, "newMap: failed to create mapEditor\n");
        return;
    }
    addToMapEditorSet(mapEditor);
    /* TODO: fix me */
    /*addWindowToSet(mapEditor->window);*/
}

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

int unsavedMapEditorAlert(MapEditor *mapEditor) {
    int response;

    if(mapEditor->mapNum) {
        response = EasyRequest(
            mapEditor->window->intuitionWindow,
            &unsavedMapAlertEasyStructWithNum,
            NULL,
            mapEditor->mapNum - 1, mapEditor->map->name);
    } else {
        response = EasyRequest(
            mapEditor->window->intuitionWindow,
            &unsavedMapAlertEasyStructNoNum,
            NULL,
            mapEditor->map->name);
    }

    switch(response) {
        case 0: return 0;                  /* cancel */
        case 1: return saveMap(mapEditor); /* save */
        case 2: return 1;                  /* don't save */
        default:
            fprintf(stderr, "unsavedMapEditorAlert: unknown response %d\n", response);
            return 0;
    }
}