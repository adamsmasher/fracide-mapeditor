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
        ASL_Window, projectWindow,
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
        projectWindow,
        &confirmRevertProjectEasyStruct,
        NULL);
}

void revertProject(void) {
    if(!confirmRevertProject()) {
        goto done;
    }

    openProjectFromFile(projectFilename);

done:
    return;
}

int saveProjectAs(void) {
    BOOL result;
    struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
        ASL_Hail, "Save Project As",
        ASL_Window, projectWindow,
        ASL_FuncFlags, FILF_SAVE,
        TAG_END);
    if(!request) {
        result = 0;
        goto done;
    }

    result = AslRequest(request, NULL);
    if(result) {
        result = saveProjectToAsl(request->rf_Dir, request->rf_File);
    }

    FreeAslRequest(request);
done:
    return result;
}

int saveProject(void) {
    if(*projectFilename) {
        if(!saveProjectToFile(projectFilename)) {
            EasyRequest(
                projectWindow,
                &projectSaveFailEasyStruct,
                NULL,
                projectFilename);
            return 0;
        }
        return 1;
    } else {
        return saveProjectAs();
    }
}

void selectTilesetPackage(void) {
    struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
        ASL_Hail, "Select Tileset Package",
        ASL_Window, projectWindow,
        TAG_END);
    if(!request) {
        fprintf(stderr, "selectTilesetPackage: failed to allocate requester\n");
        goto done;
    }

    if(AslRequest(request, NULL)) {
        if(loadTilesetPackageFromAsl(request->rf_Dir, request->rf_File)) {
            projectSaved = 0;
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

void showEntityEditor(void) {
    if(entityEditor) {
        WindowToFront(entityEditor->window);
    } else {
        entityEditor = newEntityEditor();
        if(entityEditor) {
            addWindowToSet(entityEditor->window);
        }
    }
}

void showSongNamesEditor(void) {
    if(songNamesEditor) {
        WindowToFront(songNamesEditor->window);
    } else {
        songNamesEditor = newSongNamesEditor();
        if(songNamesEditor) {
            addWindowToSet(songNamesEditor->window);
        }
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
        projectWindow,
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

        project.maps[mapNum] = allocMap();
        if(!project.maps[mapNum]) {
            fprintf(stderr, "openMapNum: failed to allocate new map\n");
            return 0;
        }
        project.mapCnt++;
        projectSaved = 0;
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
