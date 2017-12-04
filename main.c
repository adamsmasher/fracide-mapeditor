#include <proto/exec.h>

#include <proto/dos.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
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

#include "easystructs.h"
#include "EntityBrowser.h"
#include "EntityEditor.h"
#include "globals.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "menu.h"
#include "palette.h"
#include "SongNames.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"
#include "windowset.h"
#include "workspace.h"

#define SCR_WIDTH  640
#define SCR_HEIGHT 512

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

static struct Menu *menu = NULL;

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

    mapEditorSetMapNum(mapEditor, selected - 1);
    enableMapRevert(mapEditor);

    mapEditorSetSaveStatus(mapEditor, SAVED);

    updateProjectMapName(&project, selected - 1, mapEditor->map);
    projectSaved = 0;

    return 1;
}

static int saveMap(MapEditor *mapEditor) {
    if(!mapEditor->mapNum) {
        return saveMapAs(mapEditor);
    } else {
        overwriteMap(mapEditor->map, project.maps[mapEditor->mapNum - 1]);
        updateProjectMapName(&project, mapEditor->mapNum - 1, mapEditor->map);
        mapEditorSetSaveStatus(mapEditor, SAVED);
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

static int confirmRevertMap(MapEditor *mapEditor) {
    return EasyRequest(
        mapEditor->window,
        &confirmRevertMapEasyStruct,
        NULL,
        mapEditor->mapNum - 1, mapEditor->map->name);
}

static void handleProjectMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
        case IDCMP_MENUPICK:
            handleMainMenuPick(menu, msg);
    }
}

static void handleProjectMessages(void) {
    struct IntuiMessage *msg;
    while(msg = (struct IntuiMessage*)GetMsg(projectWindow->UserPort)) {
        handleProjectMessage(msg);
        ReplyMsg((struct Message*)msg);
    }
}

static int listItemStart(int selected) {
    if(selected < 10) {
        return 2;
    } else if(selected < 100) {
        return 3;
    } else {
        return 4;
    }
}

static void handleSongNamesEditorSelectSong(struct IntuiMessage *msg) {
    int selected = msg->Code;
    int i = listItemStart(selected);

    GT_SetGadgetAttrs(songNamesEditor->songNameGadget, songNamesEditor->window, NULL,
       GTST_String, &project.songNameStrs[selected][i],
       GA_Disabled, FALSE,
       TAG_END);

    songNamesEditor->selected = selected + 1;
}

static void handleSongNamesEditorUpdateSong(struct IntuiMessage *msg) {
    int selected = songNamesEditor->selected - 1;
    strcpy(
        &project.songNameStrs[selected][listItemStart(selected)],
        ((struct StringInfo*)songNamesEditor->songNameGadget->SpecialInfo)->Buffer);
    GT_RefreshWindow(songNamesEditor->window, NULL);
    projectSaved = 0;
    refreshAllSongDisplays();
}

static void handleSongNamesEditorGadgetUp(struct IntuiMessage *msg) {
    struct Gadget *gadget = (struct Gadget*)msg->IAddress;
    switch(gadget->GadgetID) {
    case SONG_LIST_ID:
        handleSongNamesEditorSelectSong(msg);
        break;
    case SONG_NAME_ID:
        handleSongNamesEditorUpdateSong(msg);
        break;
    }
}

static void handleSongNamesEditorMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        songNamesEditor->closed = 1;
        break;
    case IDCMP_GADGETUP:
        handleSongNamesEditorGadgetUp(msg);
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(songNamesEditor->window);
        GT_EndRefresh(songNamesEditor->window, TRUE);
        break;
    case IDCMP_NEWSIZE:
        resizeSongRequester(songNamesEditor);
        break;
    }
}

static void handleSongNamesEditorMessages(void) {
    struct IntuiMessage *msg;
    while(msg = GT_GetIMsg(songNamesEditor->window->UserPort)) {
        handleSongNamesEditorMessage(msg);
        GT_ReplyIMsg(msg);
    }
}

static void closeSongNamesEditor(void) {
    if(songNamesEditor) {
        removeWindowFromSet(songNamesEditor->window);
        freeSongRequester(songNamesEditor);
        songNamesEditor = NULL;
    }
}

static void handleEntityEditorSelectEntity(struct IntuiMessage *msg) {
    int selected = msg->Code;
    int i = listItemStart(selected);

    GT_SetGadgetAttrs(entityEditor->entityNameGadget, entityEditor->window, NULL,
       GTST_String, &project.entityNameStrs[selected][i],
       GA_Disabled, FALSE,
       TAG_END);

    entityEditor->selected = selected + 1;
}

static void handleEntityEditorUpdateEntity(struct IntuiMessage *msg) {
    int selected = entityEditor->selected - 1;
    strcpy(
        &project.entityNameStrs[selected][listItemStart(selected)],
        ((struct StringInfo*)entityEditor->entityNameGadget->SpecialInfo)->Buffer);
    GT_RefreshWindow(entityEditor->window, NULL);
    projectSaved = 0;
    refreshAllEntityBrowsers();
}

static void handleEntityEditorGadgetUp(struct IntuiMessage *msg) {
    struct Gadget *gadget = (struct Gadget*)msg->IAddress;
    switch(gadget->GadgetID) {
    case ENTITY_EDITOR_LIST_ID:
        handleEntityEditorSelectEntity(msg);
        break;
    case ENTITY_NAME_ID:
        handleEntityEditorUpdateEntity(msg);
        break;
    }
}

static void handleEntityEditorMessage(struct IntuiMessage* msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        entityEditor->closed = 1;
        break;
    case IDCMP_GADGETUP:
        handleEntityEditorGadgetUp(msg);
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(entityEditor->window);
        GT_EndRefresh(entityEditor->window, TRUE);
        break;
    case IDCMP_NEWSIZE:
        resizeEntityEditor(entityEditor);
        break;
    }
}

static void handleEntityEditorMessages(void) {
    struct IntuiMessage *msg;
    while(msg = GT_GetIMsg(entityEditor->window->UserPort)) {
        handleEntityEditorMessage(msg);
        GT_ReplyIMsg(msg);
    }
}

static void closeEntityEditor(void) {
    if(entityEditor) {
        removeWindowFromSet(entityEditor->window);
        freeEntityEditor(entityEditor);
        entityEditor = NULL;
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
        GT_EndRefresh(tilesetRequester->window, TRUE);
        break;
    case IDCMP_GADGETUP:
        handleTilesetRequesterGadgetUp(mapEditor, tilesetRequester, msg);
        break;
    case IDCMP_NEWSIZE:
        resizeTilesetRequester(tilesetRequester);
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

static void handleSongRequesterGadgetUp(MapEditor *mapEditor, SongRequester *songRequester, struct IntuiMessage *msg) {
    mapEditorSetSong(mapEditor, msg->Code);
}

static void handleSongRequesterMessage(MapEditor *mapEditor, SongRequester *songRequester, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        songRequester->closed = 1;
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(songRequester->window);
        GT_EndRefresh(songRequester->window, TRUE);
        break;
    case IDCMP_GADGETUP:
        handleSongRequesterGadgetUp(mapEditor, songRequester, msg);
        break;
    case IDCMP_NEWSIZE:
        resizeSongRequester(songRequester);
        break;
    }
}

static void handleSongRequesterMessages(MapEditor *mapEditor, SongRequester *songRequester) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(songRequester->window->UserPort)) {
        handleSongRequesterMessage(mapEditor, songRequester, msg);
        GT_ReplyIMsg(msg);
    }
}

static void handleAddEntityClicked(MapEditor *mapEditor, EntityBrowser *entityBrowser) {
    int newEntityIdx = mapEditor->map->entityCnt;
    Entity *entity   = &mapEditor->map->entities[newEntityIdx];

    entityBrowserFreeEntityLabels(entityBrowser);
    mapAddNewEntity(mapEditor->map);
    entityBrowserSetEntities(entityBrowser, mapEditor->map->entities, mapEditor->map->entityCnt);

    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    if(mapEditor->map->entityCnt >= MAX_ENTITIES_PER_MAP) {
        GT_SetGadgetAttrs(entityBrowser->addEntityGadget, entityBrowser->window, NULL,
            GA_Disabled, TRUE);
    }

    entityBrowserSelectEntity(entityBrowser, newEntityIdx, entity);

    mapEditorDrawEntity(mapEditor, entity, newEntityIdx);
}

static void handleRemoveEntityClicked(MapEditor *mapEditor, EntityBrowser *entityBrowser) {
    entityBrowserFreeEntityLabels(entityBrowser);
    mapRemoveEntity(mapEditor->map, entityBrowser->selectedEntity - 1);
    entityBrowserSetEntities(entityBrowser, mapEditor->map->entities, mapEditor->map->entityCnt);

    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    entityBrowserDeselectEntity(entityBrowser);

    mapEditorRefreshTileset(mapEditor);
}

static void handleEntityClicked(EntityBrowser *entityBrowser, Map *map, int entityNum) {
    Entity *entity = &map->entities[entityNum];

    if(entityBrowser->entityRequester) {
        entityBrowser->entityRequester->closed = 1;
    }

    entityBrowserSelectEntity(entityBrowser, entityNum, entity);
}

static void handleTagClicked(EntityBrowser *entityBrowser, Map *map, int tagNum) {
    Entity *entity = &map->entities[entityBrowser->selectedEntity - 1];
    Frac_tag *tag = &entity->tags[tagNum];
    entityBrowserSelectTag(entityBrowser, tagNum, tag);
}

static void handleEntityRowChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    int oldRow = entity->row;
    int oldCol = entity->col;

    entity->row = ((struct StringInfo*)entityBrowser->rowGadget->SpecialInfo)->LongInt;
    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    mapEditorDrawEntity(mapEditor, entity, entityBrowser->selectedEntity - 1);
    mapEditorRedrawTile(mapEditor, oldRow, oldCol);
}

static void handleEntityColChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    int oldRow = entity->row;
    int oldCol = entity->col;

    entity->col = ((struct StringInfo*)entityBrowser->colGadget->SpecialInfo)->LongInt;
    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    mapEditorDrawEntity(mapEditor, entity, entityBrowser->selectedEntity - 1);
    mapEditorRedrawTile(mapEditor, oldRow, oldCol);
}

static void handleEntityVRAMSlotChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    entity->vramSlot = ((struct StringInfo*)entityBrowser->VRAMSlotGadget->SpecialInfo)->LongInt;
    mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void handleAddTagClicked(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    int newTagIdx = entity->tagCnt;

    entityBrowserFreeTagLabels(entityBrowser);
    entityAddNewTag(entity);
    entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    if(entity->tagCnt >= MAX_TAGS_PER_ENTITY) {
        GT_SetGadgetAttrs(entityBrowser->addTagGadget, entityBrowser->window, NULL,
            GA_Disabled, TRUE);
    }

    entityBrowserSelectTag(entityBrowser, newTagIdx, &entity->tags[newTagIdx]);
}

static void handleDeleteTagClicked(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];

    entityBrowserFreeTagLabels(entityBrowser);
    entityDeleteTag(entity, entityBrowser->selectedTag - 1);
    entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

    mapEditorSetSaveStatus(mapEditor, UNSAVED);

    entityBrowserDeselectTag(entityBrowser);
}

static void handleTagIdChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];
    tag->id = ((struct StringInfo*)entityBrowser->tagIdGadget->SpecialInfo)->LongInt;
    mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void handleTagValueChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];
    tag->value = ((struct StringInfo*)entityBrowser->tagValueGadget->SpecialInfo)->LongInt;
    mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void handleTagAliasChanged(EntityBrowser *entityBrowser, MapEditor *mapEditor) {
    Entity *entity = &mapEditor->map->entities[entityBrowser->selectedEntity - 1];
    Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];

    entityBrowserFreeTagLabels(entityBrowser);
    strcpy(tag->alias, ((struct StringInfo*)entityBrowser->tagAliasGadget->SpecialInfo)->Buffer);
    entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

    mapEditorSetSaveStatus(mapEditor, UNSAVED);
}

static void handleChooseEntityClicked(EntityBrowser *entityBrowser) {
    EntityEditor *entityRequester = entityBrowser->entityRequester;

    if(entityRequester) {
        WindowToFront(entityRequester->window);
    } else {
        entityRequester = newEntityRequester();
        if(entityRequester) {
            attachEntityRequesterToEntityBrowser(entityBrowser, entityRequester);
            addWindowToSet(entityRequester->window);
        }
    }
}

static void handleEntityBrowserGadgetUp(MapEditor *mapEditor, EntityBrowser *entityBrowser, struct IntuiMessage *msg) {
    struct Gadget *gadget = (struct Gadget*)msg->IAddress;
    switch(gadget->GadgetID) {
    case ADD_ENTITY_ID:
        handleAddEntityClicked(mapEditor, entityBrowser);
        break;
    case REMOVE_ENTITY_ID:
        handleRemoveEntityClicked(mapEditor, entityBrowser);
        break;
    case ENTITY_BROWSER_LIST_ID:
        handleEntityClicked(entityBrowser, mapEditor->map, msg->Code);
        break;
    case CHOOSE_ENTITY_ID:
        handleChooseEntityClicked(entityBrowser);
        break;
    case TAG_LIST_ID:
        handleTagClicked(entityBrowser, mapEditor->map, msg->Code);
        break;
    case ENTITY_ROW_ID:
        handleEntityRowChanged(entityBrowser, mapEditor);
        break;
    case ENTITY_COL_ID:
        handleEntityColChanged(entityBrowser, mapEditor);
        break;
    case VRAM_SLOT_ID:
        handleEntityVRAMSlotChanged(entityBrowser, mapEditor);
        break;
    case ADD_TAG_ID:
        handleAddTagClicked(entityBrowser, mapEditor);
        break;
    case DELETE_TAG_ID:
        handleDeleteTagClicked(entityBrowser, mapEditor);
        break;
    case TAG_ID_ID:
        handleTagIdChanged(entityBrowser, mapEditor);
        break;
    case TAG_ALIAS_ID:
        handleTagAliasChanged(entityBrowser, mapEditor);
        break;
    case TAG_VALUE_ID:
        handleTagValueChanged(entityBrowser, mapEditor);
        break;
    }
}

static void handleEntityBrowserMessage(MapEditor *mapEditor, EntityBrowser *entityBrowser, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_GADGETUP:
        handleEntityBrowserGadgetUp(mapEditor, entityBrowser, msg);
        break;
    case IDCMP_CLOSEWINDOW:
        entityBrowser->closed = 1;
        break;
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(entityBrowser->window);
        GT_EndRefresh(entityBrowser->window, TRUE);
        break;
    }
}

/* TODO: close dead EntityRequesters */
static void handleEntityBrowserMessages(MapEditor *mapEditor, EntityBrowser *entityBrowser) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(entityBrowser->window->UserPort)) {
        handleEntityBrowserMessage(mapEditor, entityBrowser, msg);
        GT_ReplyIMsg(msg);
    }
}

static void handleEntityBrowserChildMessages(EntityBrowser *entityBrowser, long signalSet) {
    EntityEditor *entityRequester = entityBrowser->entityRequester;

    if(entityRequester) {
        if(1L << entityRequester->window->UserPort->mp_SigBit & signalSet) {
            /* TODO: handle entity requester messages */
        }
    }
}

static void closeAnyDeadChildrenOfMapEditor(MapEditor *mapEditor) {
    TilesetRequester *tilesetRequester = mapEditor->tilesetRequester;
    SongRequester    *songRequester    = mapEditor->songRequester;
    EntityBrowser    *entityBrowser    = mapEditor->entityBrowser;

    if(tilesetRequester && tilesetRequester->closed) {
        removeWindowFromSet(tilesetRequester->window);
        closeTilesetRequester(tilesetRequester);
        mapEditor->tilesetRequester = NULL;
    }
    if(songRequester && songRequester->closed) {
        removeWindowFromSet(songRequester->window);
        freeSongRequester(songRequester);
        mapEditor->songRequester = NULL;
    }
    if(entityBrowser && entityBrowser->closed) {
        removeWindowFromSet(entityBrowser->window);
        freeEntityBrowser(entityBrowser);
        mapEditor->entityBrowser = NULL;
    }
}

static void handleChooseTilesetClicked(MapEditor *mapEditor) {
    TilesetRequester *tilesetRequester;
    char title[32];

    if(mapEditor->tilesetRequester) {
        WindowToFront(mapEditor->tilesetRequester->window);
        return;
    }

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
    if(!tilesetPackage) {
        return;
    }

    if(mapEditor->mapNum) {
        sprintf(title, "Choose Tileset For Map %d", mapEditor->mapNum - 1);
    } else {
        strcpy(title, "Choose Tileset");
    }

    tilesetRequester = newTilesetRequester(title);
    if(!tilesetRequester) {
        fprintf(stderr, "handleChooseTilesetClicked: couldn't make requester\n");
        return;
    }

    attachTilesetRequesterToMapEditor(mapEditor, tilesetRequester);
    addWindowToSet(tilesetRequester->window);
}

static void handleChangeSongClicked(MapEditor *mapEditor) {
    if(!mapEditor->songRequester) {
        char title[32];
        SongRequester *songRequester;

        if(mapEditor->mapNum) {
            sprintf(title, "Change Soundtrack For Map %d", mapEditor->mapNum - 1);
        } else {
            strcpy(title, "Change Soundtrack");
        }

        songRequester = newSongRequester(title);
        if(songRequester) {
            attachSongRequesterToMapEditor(mapEditor, songRequester);
            addWindowToSet(songRequester->window);
        }
    } else {
        WindowToFront(mapEditor->songRequester->window);
    }
}

static void handleClearSongClicked(MapEditor *mapEditor) {
    mapEditorClearSong(mapEditor);
}

static void moveToMap(MapEditor *mapEditor, int mapNum) {
    if(mapEditor->saved || unsavedMapEditorAlert(mapEditor)) {
        if(openMapNum(mapNum - 1)) {
            mapEditor->closed = 1;
        }
    }
}

static void handleMapUp(MapEditor *mapEditor) {
    moveToMap(mapEditor, mapEditor->mapNum - 16);
}

static void handleMapDown(MapEditor *mapEditor) {
    moveToMap(mapEditor, mapEditor->mapNum + 16);
}

static void handleMapLeft(MapEditor *mapEditor) {
    moveToMap(mapEditor, mapEditor->mapNum - 1);
}

static void handleMapRight(MapEditor *mapEditor) {
    moveToMap(mapEditor, mapEditor->mapNum + 1);
}

static void openNewEntityBrowser(MapEditor *mapEditor) {
    char title[32];
    EntityBrowser *entityBrowser;

    if(mapEditor->mapNum) {
        sprintf(title, "Entities (Map %d)", mapEditor->mapNum - 1);
    } else {
        strcpy(title, "Entities");
    }

    entityBrowser = newEntityBrowser(title, mapEditor->map->entities, mapEditor->map->entityCnt);
    if(!entityBrowser) {
        fprintf(stderr, "openNewEntityBrowser: couldn't create new entity browser\n");
        goto error;
    }
    
    attachEntityBrowserToMapEditor(mapEditor, entityBrowser);
    addWindowToSet(entityBrowser->window);

    error:
        return;
}

static void handleEntitiesClicked(MapEditor *mapEditor) {
    if(!mapEditor->entityBrowser) {
        openNewEntityBrowser(mapEditor);
    } else {
        WindowToFront(mapEditor->entityBrowser->window);
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
    case SONG_CHANGE_ID:
        handleChangeSongClicked(mapEditor);
        break;
    case SONG_CLEAR_ID:
        handleClearSongClicked(mapEditor);
        break;
    case MAP_LEFT_ID:
        handleMapLeft(mapEditor);
        break;
    case MAP_RIGHT_ID:
        handleMapRight(mapEditor);
        break;
    case MAP_UP_ID:
        handleMapUp(mapEditor);
        break;
    case MAP_DOWN_ID:
        handleMapDown(mapEditor);
        break;
    case ENTITIES_ID:
        handleEntitiesClicked(mapEditor);
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

/* TODO: This should be abstracted away! */
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

static void handleMapEditorChildMessages(MapEditor *mapEditor, long signalSet) {
    TilesetRequester *tilesetRequester = mapEditor->tilesetRequester;
    SongRequester *songRequester       = mapEditor->songRequester;
    EntityBrowser *entityBrowser       = mapEditor->entityBrowser;

    if(tilesetRequester) {
        if(1L << tilesetRequester->window->UserPort->mp_SigBit & signalSet) {
            handleTilesetRequesterMessages(mapEditor, tilesetRequester);
        }
    }
    if(songRequester) {
        if(1L << songRequester->window->UserPort->mp_SigBit & signalSet) {
            handleSongRequesterMessages(mapEditor, songRequester);
        }
    }
    if(entityBrowser) {
        if(1L << entityBrowser->window->UserPort->mp_SigBit & signalSet) {
            handleEntityBrowserMessages(mapEditor, entityBrowser);
        }
        handleEntityBrowserChildMessages(entityBrowser, signalSet);
    }
}

static void handleAllMapEditorMessages(long signalSet) {
    MapEditor *i = firstMapEditor;
    while(i) {
        if(1L << i->window->UserPort->mp_SigBit & signalSet) {
            handleMapEditorMessages(i);
        }
        handleMapEditorChildMessages(i, signalSet);
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
                removeWindowFromSet(i->tilesetRequester->window);
                /* closeMapEditor takes care of everything else */
            }

            removeWindowFromSet(i->window);
            closeMapEditor(i);
        } else {
            closeAnyDeadChildrenOfMapEditor(i);
        }
        i = next;
    }
}

static void mainLoop(void) {
    long signalSet = 0;
    running = 1;
    while(running) {
        signalSet = Wait(windowSetSigMask());
        if(1L << projectWindow->UserPort->mp_SigBit & signalSet) {
            handleProjectMessages();
        }
        if(songNamesEditor) {
            if(1L << songNamesEditor->window->UserPort->mp_SigBit & signalSet) {
                handleSongNamesEditorMessages();
            }
            if(songNamesEditor->closed) {
                closeSongNamesEditor();
            }
        }
        if(entityEditor) {
            if(1L << entityEditor->window->UserPort->mp_SigBit & signalSet) {
                handleEntityEditorMessages();
            }
            if(entityEditor->closed) {
                closeEntityEditor();
            }
        }
        handleAllMapEditorMessages(signalSet);
        closeDeadMapEditors();
    }
}

int main(void) {
    int retCode;

    screen = OpenScreen(&newScreen);
    if(!screen) {
        retCode = -2;
        goto done;
    }

    initPalette(&screen->ViewPort);

/* TODO: just loop over a thing! Or something! This is dumb! */
    initMapEditorScreen();
    initMapRequesterScreen();
    initTilesetRequesterScreen();
    initSongNamesScreen();
    initEntityEditorScreen();
    initEntityBrowserScreen();

    projectNewWindow.Screen = screen;
    projectWindow = OpenWindow(&projectNewWindow);
    if(!projectWindow) {
        retCode = -3;
        goto closeScreen;
    }
    addWindowToSet(projectWindow);

    vi = GetVisualInfo(screen, TAG_END);
    if(!vi) {
        retCode = -4;
        goto closeWindow;
    }

    initMapEditorVi();
    initMapRequesterVi();
    initTilesetRequesterVi();
    initSongNamesVi();
    initEntityEditorVi();
    initEntityBrowserVi();

    menu = createMainMenu();
    if(!menu) {
        fprintf(stderr, "Error creating menu\n");
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
closeEntityEditor:
    closeEntityEditor();
closeSongNamesEditor:
    closeSongNamesEditor();
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
    removeWindowFromSet(projectWindow);
    CloseWindow(projectWindow);
closeScreen:
    CloseScreen(screen);
done:
    return retCode;
}
