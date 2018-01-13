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

#include "currentproject.h"
#include "currenttiles.h"
#include "easystructs.h"
#include "globals.h"
#include "project.h"
#include "MapEditor.h"
#include "mapeditorset.h"
#include "MapRequester.h"
#include "ProjectWindow.h"
#include "windowset.h"

static int ensureEverythingSaved(void) {
    return ensureMapEditorsSaved() && ensureProjectSaved();
}

static void updateAllTileDisplays(void) {
    MapEditor *i = mapEditorSetFirst();
    while(i) {
        if(i->tilesetRequester) {
            refreshTilesetRequesterList(i->tilesetRequester);
        }
        mapEditorRefreshTileset(i);
        i = i->next;
    }
}

void refreshAllSongDisplays(void) {
    MapEditor *i = mapEditorSetFirst();
    while(i) {
        if(i->songRequester) {
            GT_RefreshWindow(i->songRequester->window, NULL);
        }
        mapEditorRefreshSong(i);
        i = i->next;
    }
}

void refreshAllEntityBrowsers(void) {
    MapEditor *i = mapEditorSetFirst();
    while(i) {
        if(i->entityBrowser) {
            GT_RefreshWindow(i->entityBrowser->window, NULL);
        }
        i = i->next;
    }
}

void openProject(void) {
    struct FileRequester *request;

    if(!ensureEverythingSaved()) {
        goto done;
    }

    request = AllocAslRequestTags(ASL_FileRequest,
        ASL_Hail, "Open Project",
        ASL_Window, getProjectWindow(),
        TAG_END);
    if(!request) {
        goto done;
    }

    if(AslRequest(request, NULL)) {
        openProjectFromAsl(request->rf_Dir, request->rf_File);
    }

    FreeAslRequest(request);
done:
    return;
}

void newProject(void) {
    if(ensureEverythingSaved()) {
        clearProject();
        initProject(&project);
        setProjectFilename(NULL);
    }
}

static int confirmRevertProject(void) {
    return EasyRequest(
        getProjectWindow(),
        &confirmRevertProjectEasyStruct,
        NULL);
}

void revertProject(void) {
    if(!confirmRevertProject()) {
        goto done;
    }

    openProjectFromFile(getProjectFilename());

done:
    return;
}

void selectTilesetPackage(void) {
    struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
        ASL_Hail, "Select Tileset Package",
        ASL_Window, getProjectWindow(),
        TAG_END);
    if(!request) {
        fprintf(stderr, "selectTilesetPackage: failed to allocate requester\n");
        goto done;
    }

    if(AslRequest(request, NULL)) {
        if(loadTilesetPackageFromAsl(request->rf_Dir, request->rf_File)) {
            updateAllTileDisplays();
        }
    }

    FreeAslRequest(request);

done:
    return;
}

void quit(void) {
    if(ensureEverythingSaved()) {
        running = 0;
    }
}

void newMap(void) {
    MapEditor *mapEditor = newMapEditorNewMap();
    if(!mapEditor) {
        fprintf(stderr, "newMap: failed to create mapEditor\n");
        return;
    }
    addToMapEditorSet(mapEditor);
    addWindowToSet(mapEditor->window);
}

static int confirmCreateMap(int mapNum) {
    return EasyRequest(
        getProjectWindow(),
        &confirmCreateMapEasyStruct,
        NULL,
        mapNum);
}

int openMapNum(int mapNum) {
    MapEditor *mapEditor;

    if(!project.maps[mapNum]) {
        if(!confirmCreateMap(mapNum)) {
            return 0;
        }

        if(!currentProjectCreateMap(mapNum)) {
            fprintf(stderr, "openMapNum: failed to create map\n");
            return 0;
        }
    }

    mapEditor = newMapEditorWithMap(project.maps[mapNum], mapNum);
    if(!mapEditor) {
        fprintf(stderr, "openMapNum: failed to create new map editor\n");
        return 0;
    }

    addToMapEditorSet(mapEditor);
    addWindowToSet(mapEditor->window);
    enableMapRevert(mapEditor);
    return 1;
}

void openMap(void) {
    MapEditor *mapEditor;

    int selected = openMapRequester();
    if(!selected) {
        return;
    }

    mapEditor = findMapEditor(selected - 1);
    if(mapEditor) {
        WindowToFront(mapEditor->window);
    } else {
        openMapNum(selected - 1);
    }
}

int saveMapAs(MapEditor *mapEditor) {
    int selected = saveMapRequester(mapEditor);
    if(!selected) {
        return 0;
    }

    if(!project.maps[selected - 1]) {
        Map *map = copyMap(mapEditor->map);
        if(!map) {
            fprintf(stderr, "saveMapAs: failed to allocate map copy\n");
            return 0;
        }
        project.mapCnt++;
        project.maps[selected - 1] = map;
    } else {
        int response = EasyRequest(
            mapEditor->window,
            &saveIntoFullSlotEasyStruct,
            NULL,
            selected - 1, project.maps[selected - 1]->name);
        if(response) {
            overwriteMap(mapEditor->map, project.maps[selected - 1]);
        } else {
            return 0;
        }
    }

    mapEditorSetMapNum(mapEditor, selected - 1);
    enableMapRevert(mapEditor);

    mapEditorSetSaveStatus(mapEditor, SAVED);

    updateProjectMapName(&project, selected - 1, mapEditor->map);
    updateCurrentProjectMapName(selected - 1, mapEditor->map);

    return 1;
}

int saveMap(MapEditor *mapEditor) {
    if(!mapEditor->mapNum) {
        return saveMapAs(mapEditor);
    } else {
        overwriteMap(mapEditor->map, project.maps[mapEditor->mapNum - 1]);
        /* TODO: this is what sets the saved status, but that feels fragile */
        updateCurrentProjectMapName(mapEditor->mapNum - 1, mapEditor->map);
        mapEditorSetSaveStatus(mapEditor, SAVED);
        return 1;
    }
}

int unsavedMapEditorAlert(MapEditor *mapEditor) {
    int response;

    if(mapEditor->mapNum) {
        response = EasyRequest(
            mapEditor->window,
            &unsavedMapAlertEasyStructWithNum,
            NULL,
            mapEditor->mapNum - 1, mapEditor->map->name);
    } else {
        response = EasyRequest(
            mapEditor->window,
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