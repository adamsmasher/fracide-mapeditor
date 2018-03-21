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

#include "easystructs.h"
#include "EntityBrowser.h"
#include "EntityNamesEditor.h"
#include "EntityRequester.h"
#include "MapEditor.h"
#include "MapRequester.h"
#include "palette.h"
#include "ProjectWindow.h"
#include "SongNamesEditor.h"
#include "SongRequester.h"
#include "TilesetPackage.h"
#include "TilesetRequester.h"

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

static void handleTilesetRequesterGadgetUp(FrameworkWindow *mapEditorWindow, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
  mapEditorSetTileset(mapEditorWindow, msg->Code);
}

static void handleTilesetRequesterMessage(FrameworkWindow *mapEditorWindow, TilesetRequester *tilesetRequester, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_GADGETUP:
      handleTilesetRequesterGadgetUp(mapEditorWindow, tilesetRequester, msg);
      break;
    case IDCMP_NEWSIZE:
      resizeTilesetRequester(tilesetRequester);
      break;
    }
}

static void handleTilesetRequesterMessages(FrameworkWindow *mapEditorWindow, TilesetRequester *tilesetRequester) {
  struct IntuiMessage *msg = NULL;
  while(msg = GT_GetIMsg(tilesetRequester->window->intuitionWindow->UserPort)) {
    handleTilesetRequesterMessage(mapEditorWindow, tilesetRequester, msg);
    GT_ReplyIMsg(msg);
  }
}

static void handleSongRequesterGadgetUp(FrameworkWindow *mapEditorWindow, SongRequester *songRequester, struct IntuiMessage *msg) {
  mapEditorSetSong(mapEditorWindow, msg->Code);
}

static void handleSongRequesterMessage(FrameworkWindow *mapEditorWindow, SongRequester *songRequester, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_GADGETUP:
      handleSongRequesterGadgetUp(mapEditorWindow, songRequester, msg);
      break;
    case IDCMP_NEWSIZE:
      resizeSongRequester(songRequester);
      break;
  }
}

static void handleSongRequesterMessages(FrameworkWindow *mapEditorWindow, SongRequester *songRequester) {
  struct IntuiMessage *msg = NULL;
  while(msg = GT_GetIMsg(songRequester->window->intuitionWindow->UserPort)) {
    handleSongRequesterMessage(mapEditorWindow, songRequester, msg);
    GT_ReplyIMsg(msg);
  }
}

static void handleAddEntityClicked(FrameworkWindow *mapEditorWindow, EntityBrowser *entityBrowser) {
  MapEditorData *data = mapEditorWindow->data;
  int newEntityIdx = data->map->entityCnt;
  Entity *entity   = &data->map->entities[newEntityIdx];

  entityBrowserFreeEntityLabels(entityBrowser);
  mapAddNewEntity(data->map);
  entityBrowserSetEntities(entityBrowser, data->map->entities, data->map->entityCnt);

  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  if(data->map->entityCnt >= MAX_ENTITIES_PER_MAP) {
    GT_SetGadgetAttrs(entityBrowser->addEntityGadget, entityBrowser->window->intuitionWindow, NULL,
      GA_Disabled, TRUE);
  }

  entityBrowserSelectEntity(entityBrowser, newEntityIdx, entity);

  mapEditorDrawEntity(mapEditorWindow, entity, newEntityIdx);
}

static void handleRemoveEntityClicked(FrameworkWindow *mapEditorWindow, EntityBrowser *entityBrowser) {
  MapEditorData *data = mapEditorWindow->data;

  entityBrowserFreeEntityLabels(entityBrowser);
  mapRemoveEntity(data->map, entityBrowser->selectedEntity - 1);
  entityBrowserSetEntities(entityBrowser, data->map->entities, data->map->entityCnt);

  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  entityBrowserDeselectEntity(entityBrowser);

  mapEditorRefreshTileset(mapEditorWindow);
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

static void handleEntityRowChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  int oldRow = entity->row;
  int oldCol = entity->col;  

  entity->row = ((struct StringInfo*)entityBrowser->rowGadget->SpecialInfo)->LongInt;
  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  mapEditorDrawEntity(mapEditorWindow, entity, entityBrowser->selectedEntity - 1);
  mapEditorRedrawTile(mapEditorWindow, oldRow, oldCol);
}

static void handleEntityColChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  int oldRow = entity->row;
  int oldCol = entity->col;

  entity->col = ((struct StringInfo*)entityBrowser->colGadget->SpecialInfo)->LongInt;
  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  mapEditorDrawEntity(mapEditorWindow, entity, entityBrowser->selectedEntity - 1);
  mapEditorRedrawTile(mapEditorWindow, oldRow, oldCol);
}

static void handleEntityVRAMSlotChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  entity->vramSlot = ((struct StringInfo*)entityBrowser->VRAMSlotGadget->SpecialInfo)->LongInt;
  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);
}

static void handleAddTagClicked(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  int newTagIdx = entity->tagCnt;

  entityBrowserFreeTagLabels(entityBrowser);
  entityAddNewTag(entity);
  entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  if(entity->tagCnt >= MAX_TAGS_PER_ENTITY) {
    GT_SetGadgetAttrs(entityBrowser->addTagGadget, entityBrowser->window->intuitionWindow, NULL,
      GA_Disabled, TRUE);
  }

  entityBrowserSelectTag(entityBrowser, newTagIdx, &entity->tags[newTagIdx]);
}

static void handleDeleteTagClicked(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];

  entityBrowserFreeTagLabels(entityBrowser);
  entityDeleteTag(entity, entityBrowser->selectedTag - 1);
  entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);

  entityBrowserDeselectTag(entityBrowser);
}

static void handleTagIdChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];
  tag->id = ((struct StringInfo*)entityBrowser->tagIdGadget->SpecialInfo)->LongInt;
  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);
}

static void handleTagValueChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];
  tag->value = ((struct StringInfo*)entityBrowser->tagValueGadget->SpecialInfo)->LongInt;
  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);
}

static void handleTagAliasChanged(EntityBrowser *entityBrowser, FrameworkWindow *mapEditorWindow) {
  MapEditorData *data = mapEditorWindow->data;
  Entity *entity = &data->map->entities[entityBrowser->selectedEntity - 1];
  Frac_tag *tag = &entity->tags[entityBrowser->selectedTag - 1];

  entityBrowserFreeTagLabels(entityBrowser);
  strcpy(tag->alias, ((struct StringInfo*)entityBrowser->tagAliasGadget->SpecialInfo)->Buffer);
  entityBrowserSetTags(entityBrowser, entity->tags, entity->tagCnt);

  mapEditorSetSaveStatus(mapEditorWindow, UNSAVED);
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

static void handleEntityBrowserGadgetUp(FrameworkWindow *mapEditorWindow, EntityBrowser *entityBrowser, struct IntuiMessage *msg) {
  MapEditorData *data = mapEditorWindow->data;
  struct Gadget *gadget = (struct Gadget*)msg->IAddress;
  switch(gadget->GadgetID) {
    case ADD_ENTITY_ID:
      handleAddEntityClicked(mapEditorWindow, entityBrowser);
      break;
    case REMOVE_ENTITY_ID:
      handleRemoveEntityClicked(mapEditorWindow, entityBrowser);
      break;
    case ENTITY_BROWSER_LIST_ID:
      handleEntityClicked(entityBrowser, data->map, msg->Code);
      break;
    case CHOOSE_ENTITY_ID:
      handleChooseEntityClicked(entityBrowser);
      break;
    case TAG_LIST_ID:
      handleTagClicked(entityBrowser, data->map, msg->Code);
      break;
    case ENTITY_ROW_ID:
      handleEntityRowChanged(entityBrowser, mapEditorWindow);
      break;
    case ENTITY_COL_ID:
      handleEntityColChanged(entityBrowser, mapEditorWindow);
      break;
    case VRAM_SLOT_ID:
      handleEntityVRAMSlotChanged(entityBrowser, mapEditorWindow);
      break;
    case ADD_TAG_ID:
      handleAddTagClicked(entityBrowser, mapEditorWindow);
      break;
    case DELETE_TAG_ID:
      handleDeleteTagClicked(entityBrowser, mapEditorWindow);
      break;
    case TAG_ID_ID:
      handleTagIdChanged(entityBrowser, mapEditorWindow);
      break;
    case TAG_ALIAS_ID:
      handleTagAliasChanged(entityBrowser, mapEditorWindow);
      break;
    case TAG_VALUE_ID:
      handleTagValueChanged(entityBrowser, mapEditorWindow);
      break;
  }
}

static void handleEntityBrowserMessage(FrameworkWindow *mapEditorWindow, EntityBrowser *entityBrowser, struct IntuiMessage *msg) {
  switch(msg->Class) {
    case IDCMP_GADGETUP:
      handleEntityBrowserGadgetUp(mapEditorWindow, entityBrowser, msg);
      break;
    case IDCMP_CLOSEWINDOW:
      entityBrowser->closed = 1;
      break;
  }
}

/* TODO: close dead EntityRequesters */
static void handleEntityBrowserMessages(FrameworkWindow *mapEditorWindow, EntityBrowser *entityBrowser) {
  struct IntuiMessage *msg = NULL;
  while(msg = GT_GetIMsg(entityBrowser->window->intuitionWindow->UserPort)) {
    handleEntityBrowserMessage(mapEditorWindow, entityBrowser, msg);
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
  int errorCode;
  FrameworkWindow *projectWindow;

  if(!initGlobalScreen(&newScreen)) {
    errorCode = -1;
    goto error;
  }

  initPalette(getGlobalViewPort());

  projectWindow = openProjectWindow();
  if(!projectWindow) {
    errorCode = -2;
    goto error_closeScreen;
  }
    
  runMainLoop(projectWindow);

closeScreen:
  closeGlobalScreen();
done:
  return 0;

error_closeScreen:
  closeGlobalScreen();
error:
  return errorCode;
}
