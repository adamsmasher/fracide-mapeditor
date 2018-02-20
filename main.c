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

#include "framework/runstate.h"
#include "framework/screen.h"
#include "framework/windowset.h"

#include "currentproject.h"
#include "currenttiles.h"
#include "easystructs.h"
#include "EntityBrowser.h"
#include "EntityNamesEditor.h"
#include "EntityRequester.h"
#include "globals.h"
#include "MapEditor.h"
#include "mapeditorset.h"
#include "MapRequester.h"
#include "menu.h"
#include "palette.h"
#include "ProjectWindow.h"
#include "SongNamesEditor.h"
#include "SongRequester.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"
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

static void handleTilesetRequesterGadgetUp(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
  mapEditorSetTileset(mapEditor, msg->Code);
}

static void handleTilesetRequesterMessage(MapEditor *mapEditor, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
    switch(msg->Class) {
    case IDCMP_CLOSEWINDOW:
        tilesetRequester->closed = 1;
        break;
    /* TODO: the framework should handle this! */
    case IDCMP_REFRESHWINDOW:
        GT_BeginRefresh(tilesetRequester->window->intuitionWindow);
        GT_EndRefresh(tilesetRequester->window->intuitionWindow, TRUE);
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
  while(msg = GT_GetIMsg(tilesetRequester->window->intuitionWindow->UserPort)) {
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
        GT_BeginRefresh(songRequester->window->intuitionWindow);
        GT_EndRefresh(songRequester->window->intuitionWindow, TRUE);
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
    while(msg = GT_GetIMsg(songRequester->window->intuitionWindow->UserPort)) {
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
        GT_SetGadgetAttrs(entityBrowser->addEntityGadget, entityBrowser->window->intuitionWindow, NULL,
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
        GT_SetGadgetAttrs(entityBrowser->addTagGadget, entityBrowser->window->intuitionWindow, NULL,
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
    EntityRequester *entityRequester = entityBrowser->entityRequester;

    if(entityRequester) {
        WindowToFront(entityRequester->window->intuitionWindow);
    } else {
        entityRequester = newEntityRequester();
        if(entityRequester) {
            attachEntityRequesterToEntityBrowser(entityBrowser, entityRequester);
            /* TODO: fix me */
            /* addWindowToSet(entityRequester->window); */
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
        /* TODO: the framework should handle this */
        GT_BeginRefresh(entityBrowser->window->intuitionWindow);
        GT_EndRefresh(entityBrowser->window->intuitionWindow, TRUE);
        break;
    }
}

/* TODO: close dead EntityRequesters */
static void handleEntityBrowserMessages(MapEditor *mapEditor, EntityBrowser *entityBrowser) {
    struct IntuiMessage *msg = NULL;
    while(msg = GT_GetIMsg(entityBrowser->window->intuitionWindow->UserPort)) {
        handleEntityBrowserMessage(mapEditor, entityBrowser, msg);
        GT_ReplyIMsg(msg);
    }
}

static void handleEntityBrowserChildMessages(EntityBrowser *entityBrowser, long signalSet) {
    EntityRequester *entityRequester = entityBrowser->entityRequester;

    if(entityRequester) {
        if(1L << entityRequester->window->intuitionWindow->UserPort->mp_SigBit & signalSet) {
            /* TODO: handle entity requester messages */
        }
    }
}

int main(void) {
    int retCode;

    if(!initGlobalScreen(&newScreen)) {
        retCode = -1;
        goto done;
    }

    initPalette(getGlobalViewPort());

    if(!openProjectWindow()) {
        retCode = -2;
        goto closeScreen;
    }
    
    initCurrentProject();

    runMainLoop();
    
    retCode = 0;
    closeAllWindows();
freeTilesetPackage:
    freeTilesetPackage(tilesetPackage);
freeProject:
    freeCurrentProject();
closeScreen:
    closeGlobalScreen();
done:
    return retCode;
}
