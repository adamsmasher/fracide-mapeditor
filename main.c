#include <proto/exec.h>

#include <proto/dos.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <graphics/gfx.h>
#include <graphics/view.h>
#include <proto/graphics.h>

#include <libraries/asl.h>
#include <proto/asl.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

#define SCR_WIDTH  640
#define SCR_HEIGHT 512

static struct Library *intuition = NULL;

static struct NewScreen newScreen = {
    0,0,SCR_WIDTH,SCR_HEIGHT,2,
    0,3,
    HIRES|LACE,
    CUSTOMSCREEN,
    NULL,
    "FracIDE Map Editor",
    NULL,
    NULL
};

static struct NewWindow projectNewWindow = {
    0,0,SCR_WIDTH,SCR_HEIGHT,
    0xFF,0xFF,
    MENUPICK,
    BORDERLESS|BACKDROP,
    NULL,
    NULL,
    "Project",
    NULL,
    NULL,
    SCR_WIDTH,SCR_WIDTH,
    0xFFFF,0xFFFF,
    CUSTOMSCREEN
};

static struct NewMenu newMenu[] = {
    { NM_TITLE, "Project", 0, 0, 0, 0 },
        { NM_ITEM, "New",                       "N", 0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Open...",                   "O", 0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Save",                      "S", 0,               0, 0 },
        { NM_ITEM, "Save As...",                "A", 0,               0, 0 },
        { NM_ITEM, "Revert",                     0,  NM_ITEMDISABLED, 0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Select Tileset Package...",  0,  0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Quit",                      "Q", 0,               0, 0 },
    { NM_TITLE, "Maps",   0, 0, 0, 0 },
        { NM_ITEM, "New Map",     0, 0, 0, 0 },
        { NM_ITEM, "Open Map...", 0, 0, 0, 0 },
    { NM_END,   NULL,      0, 0, 0, 0 }
};
static struct Menu *menu = NULL;

static struct EasyStruct noTilesetPackageLoadedEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "No Tileset Package Loaded",
    "Cannot choose tileset when no tileset package has been loaded.",
    "Select Tileset Package...|Cancel"
};

static struct EasyStruct tilesetPackageLoadFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Loading Tileset Package",
    "Could not load tileset package from\n%s.",
    "OK"
};

static struct EasyStruct projectLoadFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Loading Project",
    "Could not load project from\n%s.",
    "OK"
};

static struct EasyStruct projectSaveFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Saving Project",
    "Could not save project to \n%s.",
    "OK"
};

static struct EasyStruct unsavedMapAlertEasyStructWithNum = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Map",
    "Save changes to map %ld, \"%s\"?",
    "Save|Don't Save|Cancel"
};

static struct EasyStruct unsavedMapAlertEasyStructNoNum = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Map",
    "Save changes to \"%s\"?",
    "Save|Don't Save|Cancel"
};

static struct EasyStruct saveIntoFullSlotEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Overwrite",
    "Map slot %ld is already occupied by \"%s\".\nAre you sure you want to overwrite it?",
    "Overwrite|Cancel"
};

static struct EasyStruct unsavedProjectAlertEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Project",
    "Some changes to this project haven't been committed to disk.\nSave changes to project?",
    "Save|Don't Save|Cancel"
};

static struct EasyStruct confirmRevertProjectEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Revert",
    "Are you sure you want to revert this project\nto the last saved version on disk?",
    "Revert|Don't Revert"
};

static struct EasyStruct confirmRevertMapEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Revert",
    "Are you sure you want to revert map %ld \"%s\"\nto the last version saved in the project?",
    "Revert|Don't Revert"
};

static int running = 0;
static long sigMask = 0;
static MapEditor *firstMapEditor = NULL;

static void addWindowToSigMask(struct Window *window) {
    sigMask |= 1L << window->UserPort->mp_SigBit;
}

static void removeWindowFromSigMask(struct Window *window) {
    sigMask &= ~(1L << window->UserPort->mp_SigBit);
}

static void addToMapEditorList(MapEditor *mapEditor) {
    mapEditor->next = firstMapEditor;
    if(firstMapEditor) {
        firstMapEditor->prev = mapEditor;
    }
    firstMapEditor = mapEditor;
}

static void removeFromMapEditorList(MapEditor *mapEditor) {
    if(mapEditor->next) {
        mapEditor->next->prev = mapEditor->prev;
    }
    if(mapEditor->prev) {
        mapEditor->prev->next = mapEditor->next;
    } else {
        firstMapEditor = mapEditor->next;
    }
}

static MapEditor *findMapEditor(int mapNum) {
    MapEditor *mapEditor = firstMapEditor;
    while(mapEditor) {
        if(mapEditor->mapNum - 1 == mapNum) {
            return mapEditor;
        }
        mapEditor = mapEditor->next;
    }
    return NULL;
}

static int loadTilesetPackageFromFile(char *file) {
    TilesetPackage *newTilesetPackage;

    newTilesetPackage = tilesetPackageLoadFromFile(file);
    if(!newTilesetPackage) {
        EasyRequest(projectWindow,
            &tilesetPackageLoadFailEasyStruct,
            NULL,
            file);
        goto error;
    }
    freeTilesetPackage(tilesetPackage);
    tilesetPackage = newTilesetPackage;
    strcpy(project.tilesetPackagePath, file);

    return 1;

error:
    return 0;
}

static int loadTilesetPackageFromAsl(char *dir, char *file) {
    char buffer[TILESET_PACKAGE_PATH_SIZE];

    if(strlen(dir) >= sizeof(buffer)) {
        fprintf(stderr, "loadTilesetPackageFromAsl: dir %s file %s doesn't fit in buffer\n", dir, file);
        goto error;
    }

    strcpy(buffer, dir);
    if(!AddPart(buffer, file, TILESET_PACKAGE_PATH_SIZE)) {
        fprintf(stderr, "loadTilesetPackageFromAsl: dir %s file %s doesn't fit in buffer\n", dir, file);
        goto error;
    }

    return loadTilesetPackageFromFile(buffer);

error:
    return 0;
}

static void closeAllMapEditors(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        MapEditor *next = i->next;
        removeWindowFromSigMask(i->window);
        if(i->tilesetRequester) {
            removeWindowFromSigMask(i->tilesetRequester->window);
        }
        closeMapEditor(i);
        i = next;
    }
    firstMapEditor = NULL;
}

#define REVERT_PROJECT_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static void setProjectFilename(char *filename) {
    if(filename) {
        strcpy(projectFilename, filename);
        OnMenu(projectWindow, REVERT_PROJECT_MENU_ITEM);
    } else {
        projectFilename[0] = '\0';
        OffMenu(projectWindow, REVERT_PROJECT_MENU_ITEM);
    }
}

#define REVERT_MAP_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static void enableMapRevert(MapEditor *mapEditor) {
    OnMenu(mapEditor->window, REVERT_MAP_MENU_ITEM);
}

static void disableMapRevert(MapEditor *mapEditor) {
    OffMenu(mapEditor->window, REVERT_MAP_MENU_ITEM);
}

static void clearProject(void) {
    closeAllMapEditors();
    freeTilesetPackage(tilesetPackage);
    tilesetPackage = NULL;
    freeProject(&project);
    projectSaved = 1;
}

static void newProject(void) {
    clearProject();
    initProject(&project);
    setProjectFilename(NULL);
}

static void openProjectFromFile(char *file) {
    Project *myNewProject;

    myNewProject = malloc(sizeof(Project));
    if(!myNewProject) {
        fprintf(stderr, "openProjectFromFile: failed to allocate project\n");
        goto done;
    }

    if(!loadProjectFromFile(file, myNewProject)) {
        EasyRequest(
            projectWindow,
            &projectLoadFailEasyStruct,
            NULL,
            file);
        goto freeProject;
    }

    clearProject();
    copyProject(myNewProject, &project);
    setProjectFilename(file);

    if(*project.tilesetPackagePath && !loadTilesetPackageFromFile(project.tilesetPackagePath)) {
        EasyRequest(
            projectWindow,
            &tilesetPackageLoadFailEasyStruct,
            NULL,
            project.tilesetPackagePath);

        /* because the tileset will now be empty, we've changed from the
           saved version */
        projectSaved = 0;
    }

freeProject:
    free(myNewProject);
done:
    return;
}

static void openProjectFromAsl(char *dir, char *file) {
    size_t bufferLen = strlen(dir) + strlen(file) + 2;
    char *buffer = malloc(bufferLen);

    if(!buffer) {
        fprintf(
            stderr,
            "openProjectFromAsl: failed to allocate buffer "
            "(dir: %s) (file: %s)\n",
            dir  ? dir  : "NULL",
            file ? file : "NULL");
        goto done;
    }

    strcpy(buffer, dir);
    if(!AddPart(buffer, file, (ULONG)bufferLen)) {
        fprintf(
            stderr,
            "openProjectFromAsl: failed to add part "
            "(buffer: %s) (file: %s) (len: %d)\n",
            buffer ? buffer : "NULL",
            file   ? file   : "NULL",
            bufferLen);
        goto freeBuffer;
    }

    openProjectFromFile(buffer);

freeBuffer:
    free(buffer);
done:
    return;
}

static int saveMapAs(MapEditor *mapEditor) {
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

    mapEditor->mapNum = selected;
    enableMapRevert(mapEditor);

    mapEditor->saved  = 1;

    updateProjectMapName(&project, selected - 1, mapEditor->map);
    projectSaved = 0;

    return 1;
}

static int saveMap(MapEditor *mapEditor) {
    if(!mapEditor->mapNum) {
        return saveMapAs(mapEditor);
    } else {
        overwriteMap(mapEditor->map, project.maps[mapEditor->mapNum - 1]);
        mapEditor->saved = 1;
        projectSaved = 0;
        return 1;
    }
}

static int unsavedMapEditorAlert(MapEditor *mapEditor) {
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

static int ensureMapEditorsSaved(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        if(!i->saved && !unsavedMapEditorAlert(i)) {
            return 0;
        }
        i = i->next;
    }
    return 1;
}

static int saveProjectToAsl(char *dir, char *file) {
    int result;
    size_t bufferLen = strlen(dir) + strlen(file) + 2;
    char *buffer = malloc(bufferLen);

    if(!buffer) {
        fprintf(
            stderr,
            "saveProjectToAsl: failed to allocate buffer "
            "(dir: %s) (file: %s)\n",
            dir  ? dir  : "NULL",
            file ? file : "NULL");
        result = 0;
        goto done;
    }

    strcpy(buffer, dir);
    if(!AddPart(buffer, file, (ULONG)bufferLen)) {
        fprintf(
            stderr,
            "saveProjectToAsl: failed to add part "
            "(buffer: %s) (file: %s) (len: %d)\n",
            buffer ? buffer : "NULL",
            file   ? file   : "NULL",
            bufferLen);
        result = 0;
        goto freeBuffer;
    }

    if(!saveProjectToFile(buffer)) {
        EasyRequest(projectWindow,
            &projectSaveFailEasyStruct,
            NULL,
            buffer);
        result = 0;
        goto freeBuffer;
    }
    setProjectFilename(buffer);

    projectSaved = 1;
    result = 1;

freeBuffer:
    free(buffer);
done:
    return result;
}

static int saveProjectAs(void) {
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

static int saveProject(void) {
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

static int unsavedProjectAlert(void) {
    int response = EasyRequest(
        projectWindow,
        &unsavedProjectAlertEasyStruct,
        NULL);

    switch(response) {
        case 0: return 0;
        case 1: return saveProject();
        case 2: return 1;
        default:
            fprintf(stderr, "unsavedProjectAlert: unknown response %d\n", response);
            return 0;
    }
}

static int ensureProjectSaved(void) {
    return projectSaved || unsavedProjectAlert();
}

static int ensureEverythingSaved(void) {
    return ensureMapEditorsSaved() && ensureProjectSaved();
}

static void openProject(void) {
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

static int confirmRevertProject(void) {
    return EasyRequest(
        projectWindow,
        &confirmRevertProjectEasyStruct,
        NULL);
}

static int confirmRevertMap(MapEditor *mapEditor) {
    return EasyRequest(
        mapEditor->window,
        &confirmRevertMapEasyStruct,
        NULL,
        mapEditor->mapNum - 1, mapEditor->map->name);
}

static void revertProject(void) {
    if(!confirmRevertProject()) {
        goto done;
    }

    openProjectFromFile(projectFilename);

done:
    return;
}

static void selectTilesetPackage(void) {
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
        }
    }

    FreeAslRequest(request);

done:
    return;
}

static void handleProjectMenuPick(UWORD itemNum, UWORD subNum) {
    switch(itemNum) {
        case 0:
            if(ensureEverythingSaved()) {
                newProject();
            }
            break;
        case 2: openProject(); break;
        case 4: saveProject(); break;
        case 5: saveProjectAs(); break;
        case 6: revertProject(); break;
        case 8: selectTilesetPackage(); break;
        case 10:
            if(ensureEverythingSaved()) {
                running = 0;
            }
            break;
    }
}

static void newMap(void) {
    MapEditor *mapEditor = newMapEditorNewMap();
    if(!mapEditor) {
        fprintf(stderr, "newMap: failed to create mapEditor\n");
        return;
    }
    addToMapEditorList(mapEditor);
    addWindowToSigMask(mapEditor->window);
}

static void openMapNum(int mapNum) {
    MapEditor *mapEditor =
        newMapEditorWithMap(project.maps[mapNum], mapNum);
    if(!mapEditor) {
        fprintf(stderr, "openMapNum: failed to create new map editor\n");
        return;
    }
    addToMapEditorList(mapEditor);
    addWindowToSigMask(mapEditor->window);
    enableMapRevert(mapEditor);
}

static void openMap(void) {
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

static void handleMapsMenuPick(UWORD itemNum, UWORD subNum) {
    switch(itemNum) {
        case 0: newMap(); break;
        case 1: openMap(); break;
    }
}

static void handleMainMenuPick(ULONG menuNumber) {
    UWORD menuNum = MENUNUM(menuNumber);
    UWORD itemNum = ITEMNUM(menuNumber);
    UWORD subNum  = SUBNUM(menuNumber);
    switch(menuNum) {
        case 0: handleProjectMenuPick(itemNum, subNum); break;
        case 1: handleMapsMenuPick(itemNum, subNum); break;
    }
}

static void handleMainMenuPicks(ULONG menuNumber) {
    struct MenuItem *item = NULL;
    while(running && menuNumber != MENUNULL) {
        handleMainMenuPick(menuNumber);
        item = ItemAddress(menu, menuNumber);
        menuNumber = item->NextSelect;
    }
}

static void handleProjectMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
        case IDCMP_MENUPICK:
            handleMainMenuPicks((ULONG)msg->Code);
    }
}

static void handleProjectMessages(void) {
    struct IntuiMessage *msg;
    while(msg = (struct IntuiMessage*)GetMsg(projectWindow->UserPort)) {
        handleProjectMessage(msg);
        ReplyMsg((struct Message*)msg);
    }
}

static void handleChooseTilesetClicked(MapEditor *mapEditor) {
    if(!tilesetPackage) {
        int choice = EasyRequest(
            mapEditor->window,
            &noTilesetPackageLoadedEasyStruct,
            NULL);
        if(choice) {
            selectTilesetPackage();
        }
    }

    /* even after giving the user the opportunity to set the tileset
       package, we need to be sure they did so... */
    if(tilesetPackage && !mapEditor->tilesetRequester) {
        TilesetRequester *tilesetRequester = newTilesetRequester();
        if(tilesetRequester) {
            attachTilesetRequesterToMapEditor(mapEditor, tilesetRequester);
            addWindowToSigMask(tilesetRequester->window);
        }
    }
}

static void handleMapEditorGadgetUp
(MapEditor *mapEditor, struct Gadget *gadget) {
    switch(gadget->GadgetID) {
    case CHOOSE_TILESET_ID:
        handleChooseTilesetClicked(mapEditor);
        break;
    case MAP_NAME_ID:
        updateMapEditorMapName(mapEditor);
        break;
    }
}

static void handleMapEditorPaletteClick(MapEditor *mapEditor, WORD x, WORD y) {
    int tile = mapEditorGetPaletteTileClicked(x, y);
    mapEditorSetSelected(mapEditor, tile);
}

static void handleMapEditorMapClick(MapEditor *mapEditor, WORD x, WORD y) {
    unsigned int tile = mapEditorGetMapTileClicked(x, y);
    mapEditorSetTile(mapEditor, tile);
}

static void handleMapEditorClick(MapEditor *mapEditor, WORD x, WORD y) {
    if(mapEditor->map->tilesetNum) {
        if(mapEditorClickInPalette(x, y)) {
            handleMapEditorPaletteClick(mapEditor, x, y);
        } else if(mapEditorClickInMap(x, y)) {
            handleMapEditorMapClick(mapEditor, x, y);
        }
    }
}

static void revertMap(MapEditor *mapEditor) {
    if(confirmRevertMap(mapEditor)) {
        mapEditor->closed = 1;
        openMapNum(mapEditor->mapNum - 1);
    }
}

static void handleMapMenuPick(MapEditor *mapEditor, UWORD itemNum, UWORD subNum) {
    switch(itemNum) {
        case 0: newMap(); break;
        case 2: openMap(); break;
        case 4: saveMap(mapEditor); break;
        case 5: saveMapAs(mapEditor); break;
        case 6: revertMap(mapEditor); break;
        case 8:
            if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
                mapEditor->closed = 1;
            };
            break;
    }
}

static void handleMapEditorMenuPick(MapEditor *mapEditor, ULONG menuNumber) {
    UWORD menuNum = MENUNUM(menuNumber);
    UWORD itemNum = ITEMNUM(menuNumber);
    UWORD subNum  = SUBNUM(menuNumber);
    switch(menuNum) {
        case 0: handleMapMenuPick(mapEditor, itemNum, subNum); break;
    }
}

static void handleMapEditorMenuPicks(MapEditor *mapEditor, ULONG menuNumber) {
    struct MenuItem *item = NULL;
    while(!mapEditor->closed && menuNumber != MENUNULL) {
        handleMapEditorMenuPick(mapEditor, menuNumber);
        item = ItemAddress(menu, menuNumber);
        menuNumber = item->NextSelect;
    }
}

static void handleMapEditorMessage(MapEditor *mapEditor, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
            mapEditor->closed = 1;
        }
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(mapEditor->window);
        refreshMapEditor(mapEditor);
        GT_EndRefresh(mapEditor->window, TRUE);
        break;
    case IDCMP_GADGETUP:
        handleMapEditorGadgetUp(mapEditor, (struct Gadget*)msg->IAddress);
        break;
    case IDCMP_MOUSEBUTTONS:
        handleMapEditorClick(mapEditor, msg->MouseX, msg->MouseY);
        break;
    case IDCMP_MENUPICK:
        handleMapEditorMenuPick(mapEditor, (ULONG)msg->Code);
        break;
    }
}

static void handleMapEditorMessages(MapEditor *mapEditor) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(mapEditor->window->UserPort)) {
        handleMapEditorMessage(mapEditor, msg);
        GT_ReplyIMsg(msg);
    }
}

static void handleAllMapEditorMessages(long signalSet) {
    MapEditor *i = firstMapEditor;
    while(i) {
        if(1L << i->window->UserPort->mp_SigBit & signalSet) {
            handleMapEditorMessages(i);
        }
        i = i->next;
    }
}

static void closeDeadMapEditors(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        MapEditor *next = i->next;
        if(i->closed) {
            if(i->next) {
                i->next->prev = i->prev;
            }
            if(i->prev) {
                i->prev->next = i->next;
            } else {
                firstMapEditor = next;
            }

            if(i->tilesetRequester) {
                removeWindowFromSigMask(i->tilesetRequester->window);
                /* closeMapEditor takes care of everything else */
            }

            removeWindowFromSigMask(i->window);
            closeMapEditor(i);
        }
        i = next;
    }
}

static void handleTilesetRequesterGadgetUp(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
    mapEditorSetTileset(mapEditor, msg->Code);
}

static void handleTilesetRequesterMessage(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        tilesetRequester->closed = 1;
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(tilesetRequester->window);
        refreshTilesetRequester(tilesetRequester);
        GT_EndRefresh(tilesetRequester->window, TRUE);
        break;
    case IDCMP_GADGETUP:
        handleTilesetRequesterGadgetUp(mapEditor, tilesetRequester, msg);
        break;
    }
}

static void handleTilesetRequesterMessages(MapEditor *mapEditor, TilesetRequester *tilesetRequester) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(tilesetRequester->window->UserPort)) {
        handleTilesetRequesterMessage(mapEditor, tilesetRequester, msg);
        GT_ReplyIMsg(msg);
    }
}

static void handleAllTilesetRequesterMessages(long signalSet) {
    MapEditor *i = firstMapEditor;
    while(i) {
        TilesetRequester *tilesetRequester = i->tilesetRequester;
        if(tilesetRequester) {
            if(1L << tilesetRequester->window->UserPort->mp_SigBit & signalSet) {
                handleTilesetRequesterMessages(i, tilesetRequester);
            }
        }
        i = i->next;
    }
}

static void closeDeadTilesetRequesters(void) {
    MapEditor *i = firstMapEditor;
    while(i) {
        if(i->tilesetRequester && i->tilesetRequester->closed) {
            removeWindowFromSigMask(i->tilesetRequester->window);
            closeTilesetRequester(i->tilesetRequester);
            i->tilesetRequester = NULL;
        }
        i = i->next;
    }
}

static void mainLoop(void) {
    long signalSet = 0;
    running = 1;
    while(running) {
        signalSet = Wait(sigMask);
        if(1L << projectWindow->UserPort->mp_SigBit & signalSet) {
            handleProjectMessages();
        }
        handleAllMapEditorMessages(signalSet);
        handleAllTilesetRequesterMessages(signalSet);
        closeDeadMapEditors();
        closeDeadTilesetRequesters();
    }
}

static void initPalette(struct ViewPort *viewport) {
    LONG i;
    ULONG c = 15;
    for(i = 0; i < 4; i++, c -= 5) {
        SetRGB4(viewport, i, c, c, c);
    }
}

int main(void) {
    int retCode;
    
    intuition =    OpenLibrary("intuition.library", 0);
    if(!intuition) {
        retCode = -1;
        goto done;
    }

    screen = OpenScreen(&newScreen);
    if(!screen) {
        retCode = -2;
        goto closeIntuition;
    }
    
    initPalette(&screen->ViewPort);

    initMapEditorScreen();
    initMapRequesterScreen();
    initTilesetRequesterScreen();

    projectNewWindow.Screen = screen;
    projectWindow = OpenWindow(&projectNewWindow);
    if(!projectWindow) {
        retCode = -3;
        goto closeScreen;
    }
    addWindowToSigMask(projectWindow);

    vi = GetVisualInfo(screen, TAG_END);
    if(!vi) {
        retCode = -4;
        goto closeWindow;
    }

    initMapEditorVi();
    initMapRequesterVi();
    initTilesetRequesterVi();

    menu = CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
    if(!menu) {
        retCode = -5;
        goto freeVisualInfo;
    }

    if(!LayoutMenus(menu, vi, TAG_END)) {
        retCode = -6;
        goto freeMenu;

    }

    if(!initMapEditorMenu()) {
        retCode = -7;
        goto freeMenu;
    }


    SetMenuStrip(projectWindow, menu);

    ActivateWindow(projectWindow);
    
    initProject(&project);

    mainLoop();
    
    retCode = 0;
closeAllMapEditors:
    closeAllMapEditors();
freeTilesetPackage:
    freeTilesetPackage(tilesetPackage);
freeProject:
    freeProject(&project);
clearMenu:
    ClearMenuStrip(projectWindow);
freeMapEditorMenu:
    freeMapEditorMenu();
freeMenu:
    FreeMenus(menu);
freeVisualInfo:
    FreeVisualInfo(vi);
closeWindow:
    removeWindowFromSigMask(projectWindow);
    CloseWindow(projectWindow);
closeScreen:
    CloseScreen(screen);
closeIntuition:
    CloseLibrary(intuition);
done:
    return retCode;
}
