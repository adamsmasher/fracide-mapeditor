#include "EntityBrowser.h"

#include <proto/exec.h>

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/font.h"
#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "map.h"
#include "MapEditor.h"
#include "MapEditorData.h"
#include "Result.h"

#define ENTITY_BROWSER_WIDTH  350
#define ENTITY_BROWSER_HEIGHT 225

#define ENTITY_LIST_WIDTH  120
#define ENTITY_LIST_HEIGHT 170
#define ENTITY_LIST_LEFT   10
#define ENTITY_LIST_TOP    20

#define ADD_ENTITY_WIDTH   120
#define ADD_ENTITY_HEIGHT  14
#define ADD_ENTITY_LEFT    10
#define ADD_ENTITY_TOP     190

#define REMOVE_ENTITY_WIDTH  120
#define REMOVE_ENTITY_HEIGHT 14
#define REMOVE_ENTITY_LEFT   10
#define REMOVE_ENTITY_TOP    205

#define THIS_ENTITY_WIDTH    102
#define THIS_ENTITY_HEIGHT   14
#define THIS_ENTITY_LEFT     200
#define THIS_ENTITY_TOP      25

#define CHOOSE_ENTITY_WIDTH  32
#define CHOOSE_ENTITY_HEIGHT 14
#define CHOOSE_ENTITY_LEFT   300
#define CHOOSE_ENTITY_TOP    25

#define ENTITY_ROW_WIDTH     32
#define ENTITY_ROW_HEIGHT    14
#define ENTITY_ROW_LEFT      175
#define ENTITY_ROW_TOP       50

#define ENTITY_COL_WIDTH     32
#define ENTITY_COL_HEIGHT    14
#define ENTITY_COL_LEFT      300
#define ENTITY_COL_TOP       50

#define VRAM_SLOT_WIDTH      48
#define VRAM_SLOT_HEIGHT     14
#define VRAM_SLOT_LEFT       223
#define VRAM_SLOT_TOP        75

#define TAG_LIST_WIDTH       190
#define TAG_LIST_HEIGHT      50
#define TAG_LIST_LEFT        143
#define TAG_LIST_TOP         100

#define ADD_TAG_WIDTH        190
#define ADD_TAG_HEIGHT       14
#define ADD_TAG_LEFT         143
#define ADD_TAG_TOP          150

#define DELETE_TAG_WIDTH     190
#define DELETE_TAG_HEIGHT    14
#define DELETE_TAG_LEFT      143
#define DELETE_TAG_TOP       165

#define TAG_ALIAS_WIDTH      143
#define TAG_ALIAS_HEIGHT     14
#define TAG_ALIAS_LEFT       190
#define TAG_ALIAS_TOP        185

#define TAG_ID_WIDTH         48
#define TAG_ID_HEIGHT        14
#define TAG_ID_LEFT          165
#define TAG_ID_TOP           205

#define TAG_VALUE_WIDTH      48
#define TAG_VALUE_HEIGHT     14
#define TAG_VALUE_LEFT       285
#define TAG_VALUE_TOP        205

static Result createEntityLabels(FrameworkWindow *entityBrowser) {
  /* TODO: free the old labels if necessary */
  EntityBrowserData *data = entityBrowser->data;
  MapEditorData *mapEditorData = entityBrowser->parent->data;
  UWORD entityCount = mapEditorDataGetEntityCount(mapEditorData);
  int i;

  NewList(&data->entityLabels);

  if(!entityCount) {
    data->entityNodes   = NULL;
    data->entityStrings = NULL;
    goto ok;
  }

  data->entityNodes = malloc(sizeof(struct Node) * entityCount);
  if(!data->entityNodes) {
    fprintf(stderr, "createEntityLabels: couldn't allocate memory for %d nodes\n", entityCount);
    goto error;
  }

  data->entityStrings = malloc(ENTITY_LABEL_LENGTH * entityCount);
  if(!data->entityStrings) {
    fprintf(stderr, "createEntityLabels: couldn't allocate memory for labels\n");
    goto error_freeNodes;
  }

  for(i = 0; i < entityCount; i++) {
    sprintf(data->entityStrings[i], "%d: N/A", i);
    data->entityNodes[i].ln_Name = data->entityStrings[i];
    AddTail(&data->entityLabels, &data->entityNodes[i]);
  }

ok:
  return SUCCESS;

error_freeNodes:
  free(data->entityNodes);
  data->entityNodes = NULL;
error:
  data->entityStrings = NULL;
  return FAILURE;
}

static Result entityBrowserRefreshEntities(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;

  if(!createEntityLabels(entityBrowser)) {
    fprintf(stderr, "entityBrowserRefreshEntities: couldn't create entity labels\n");
    goto error;
  }

  GT_SetGadgetAttrs(data->entityListGadget, entityBrowser->intuitionWindow, NULL,
    GTLV_Labels, &data->entityLabels,
    TAG_END);

  return SUCCESS;
error:
  return FAILURE;
}

static Result createTagLabels(FrameworkWindow *entityBrowser) {
  /* TODO: free the old labels if necessary */
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  MapEditorData *mapEditorData = parent->data;
  int tagCnt = data->selectedEntity ? 
    mapEditorDataEntityGetTagCount(mapEditorData, data->selectedEntity - 1) :
    0;
  int i;

  NewList(&data->tagLabels);

  if(!tagCnt) {
    data->tagNodes   = NULL;
    data->tagStrings = NULL;
    goto ok;
  }

  data->tagNodes = malloc(sizeof(struct Node) * tagCnt);
  if(!data->tagNodes) {
    fprintf(stderr, "createTagLabels: couldn't allocate memory for %d nodes\n", tagCnt);
    goto error;
  }

  data->tagStrings = malloc((TAG_ALIAS_LENGTH + 4) * tagCnt);
  if(!data->tagStrings) {
    fprintf(stderr, "createTagLabels: couldn't allocate memory for labels\n");
    goto error_freeNodes;
  }

  for(i = 0; i < tagCnt; i++) {
    sprintf(data->tagStrings[i], "%d: %s", i, mapEditorDataEntityGetTagAlias(mapEditorData, data->selectedEntity - 1, i));
    data->tagNodes[i].ln_Name = data->tagStrings[i];
    AddTail(&data->tagLabels, &data->tagNodes[i]);
  }

ok:
  return SUCCESS;

error_freeNodes:
  free(data->tagNodes);
  data->tagNodes = NULL;
error:
  data->tagStrings = NULL;
  return 0;
}

static Result entityBrowserRefreshTags(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;

  if(!createTagLabels(entityBrowser)) {
    fprintf(stderr, "entityBrowserRefreshTags: couldn't create tag labels\n");
    goto error;
  }

  GT_SetGadgetAttrs(data->tagListGadget, entityBrowser->intuitionWindow, NULL,
    GTLV_Labels, &data->tagLabels,
    TAG_END);

  return SUCCESS;
error:
  return FAILURE;
}

static void entityBrowserSelectTag(FrameworkWindow *entityBrowser, UWORD entityNum, int tagNum) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  MapEditorData *mapEditorData = parent->data;

  data->selectedTag = tagNum + 1;

  GT_SetGadgetAttrs(data->deleteTagGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(data->tagAliasGadget, entityBrowser->intuitionWindow, NULL,
    GTST_String, mapEditorDataEntityGetTagAlias(mapEditorData, entityNum, tagNum),
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(data->tagIdGadget, entityBrowser->intuitionWindow, NULL,
    GTIN_Number, mapEditorDataEntityGetTagId(mapEditorData, entityNum, tagNum),
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(data->tagValueGadget, entityBrowser->intuitionWindow, NULL,
    GTIN_Number, mapEditorDataEntityGetTagValue(mapEditorData, entityNum, tagNum),
    GA_Disabled, FALSE,
    TAG_END);
}

static void entityBrowserDeselectTag(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;

  GT_SetGadgetAttrs(data->tagAliasGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->tagIdGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->tagValueGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->deleteTagGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  data->selectedTag = 0;
}

static void freeEntityLabels(EntityBrowserData *data) {
  free(data->entityNodes);
  free(data->entityStrings);
}

static void freeTagLabels(EntityBrowserData *data) {
  free(data->tagNodes);
  free(data->tagStrings);
}

static void entityBrowserFreeTagLabels(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;

  GT_SetGadgetAttrs(data->tagListGadget, entityBrowser->intuitionWindow, NULL,
    GTLV_Labels, NULL,
    TAG_END);

  freeTagLabels(data);
}

static void entityBrowserSelectEntity(FrameworkWindow *entityBrowser, int entityNum) {
  EntityBrowserData *data = entityBrowser->data;
  MapEditorData *mapEditorData = entityBrowser->parent->data;
  
  data->selectedEntity = entityNum + 1;

  GT_SetGadgetAttrs(data->removeEntityGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    TAG_END);

  GT_SetGadgetAttrs(data->rowGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    GTIN_Number, mapEditorDataGetEntityRow(mapEditorData, entityNum),
    TAG_END);

  GT_SetGadgetAttrs(data->colGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    GTIN_Number, mapEditorDataGetEntityCol(mapEditorData, entityNum),
    TAG_END);

  GT_SetGadgetAttrs(data->VRAMSlotGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    GTIN_Number, mapEditorDataGetEntityVRAMSlot(mapEditorData, entityNum),
    TAG_END);

  GT_SetGadgetAttrs(data->addTagGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, mapEditorDataEntityGetTagCount(mapEditorData, entityNum) >= MAX_TAGS_PER_ENTITY,
    TAG_END);

  GT_SetGadgetAttrs(data->chooseEntityGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, FALSE,
    TAG_END);

  entityBrowserRefreshTags(entityBrowser);
  entityBrowserDeselectTag(entityBrowser);
}

static void entityBrowserDeselectEntity(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  data->selectedEntity = 0;

  GT_SetGadgetAttrs(data->removeEntityGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->rowGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->colGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->VRAMSlotGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->addTagGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  GT_SetGadgetAttrs(data->chooseEntityGadget, entityBrowser->intuitionWindow, NULL,
    GA_Disabled, TRUE,
    TAG_END);

  entityBrowserDeselectTag(entityBrowser);
  entityBrowserFreeTagLabels(entityBrowser);
}

static void onAddEntityClick(FrameworkWindow *entityBrowser) {
  EntityBrowserData *entityBrowserData = entityBrowser->data;
  FrameworkWindow *mapEditor = entityBrowser->parent;
  MapEditorData *mapEditorData = mapEditor->data;
  UWORD entityCount = mapEditorDataGetEntityCount(mapEditorData);
  UWORD newEntityIdx;

  mapEditorDataAddNewEntity(mapEditorData);
  newEntityIdx = entityCount++;
  entityBrowserRefreshEntities(entityBrowser);

  if(entityCount >= MAX_ENTITIES_PER_MAP) {
    GT_SetGadgetAttrs(entityBrowserData->addEntityGadget, entityBrowser->intuitionWindow, NULL,
      GA_Disabled, TRUE);
  }

  entityBrowserSelectEntity(entityBrowser, newEntityIdx);
}

static void onRemoveEntityClick(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *mapEditor = entityBrowser->parent;

  mapEditorDataRemoveEntity(mapEditor->data, data->selectedEntity - 1);
  entityBrowserRefreshEntities(entityBrowser);

  entityBrowserDeselectEntity(entityBrowser);
}

static void handleEntityClicked(FrameworkWindow *entityBrowser, int entityNum) {
  EntityBrowserData *data = entityBrowser->data;

  if(data->entityRequester) {
    data->entityRequester->closed = 1;
  }

  entityBrowserSelectEntity(entityBrowser, entityNum);
}

static void handleTagClicked(FrameworkWindow *entityBrowser, int tagNum) {
  EntityBrowserData *data = entityBrowser->data;
  UWORD entityNum = data->selectedEntity - 1;
  entityBrowserSelectTag(entityBrowser, entityNum, tagNum);
}

static void onEntityRowEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  UBYTE newRow = ((struct StringInfo*)data->rowGadget->SpecialInfo)->LongInt;

  mapEditorDataSetEntityRow(parent->data, data->selectedEntity - 1, newRow);
}

static void onEntityColEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  UBYTE newCol = ((struct StringInfo*)data->colGadget->SpecialInfo)->LongInt;

  mapEditorDataSetEntityCol(parent->data, data->selectedEntity - 1, newCol);
}

static void onVRAMSlotEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  UBYTE vramSlot = ((struct StringInfo*)data->VRAMSlotGadget->SpecialInfo)->LongInt;

  mapEditorDataSetEntityVRAMSlot(parent->data, data->selectedEntity - 1, vramSlot);
}

static void onAddTagClick(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  MapEditorData *mapEditorData = parent->data;
  UWORD entityIdx = data->selectedEntity - 1;
  UWORD entityTagCount = mapEditorDataEntityGetTagCount(mapEditorData, entityIdx);
  UWORD newTagIdx;

  mapEditorDataEntityAddNewTag(mapEditorData, entityIdx);
  newTagIdx = entityTagCount++;
  entityBrowserRefreshTags(entityBrowser);

  if(entityTagCount >= MAX_TAGS_PER_ENTITY) {
    GT_SetGadgetAttrs(data->addTagGadget, entityBrowser->intuitionWindow, NULL,
      GA_Disabled, TRUE);
  }

  entityBrowserSelectTag(entityBrowser, entityIdx, newTagIdx);
}

static void onDeleteTagClick(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;

  mapEditorDataEntityDeleteTag(parent->data, data->selectedEntity - 1, data->selectedTag - 1);
  entityBrowserRefreshTags(entityBrowser);
  entityBrowserDeselectTag(entityBrowser);
}

static void onTagIdEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  UBYTE newTagId = ((struct StringInfo*)data->tagIdGadget->SpecialInfo)->LongInt;
  mapEditorDataEntitySetTagId(parent->data, data->selectedEntity - 1, data->selectedTag - 1, newTagId);
}

static void onTagValueEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  UBYTE newTagValue = ((struct StringInfo*)data->tagValueGadget->SpecialInfo)->LongInt;
  mapEditorDataEntitySetTagValue(parent->data, data->selectedEntity - 1, data->selectedTag - 1, newTagValue);
}

static void onTagAliasEntry(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  FrameworkWindow *parent = entityBrowser->parent;
  const char *newTagAlias = ((struct StringInfo*)data->tagAliasGadget->SpecialInfo)->Buffer;
  mapEditorDataEntitySetTagAlias(parent->data, data->selectedEntity - 1, data->selectedTag - 1, newTagAlias);
  entityBrowserRefreshTags(entityBrowser);
}

static void onChooseEntityClick(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  EntityRequester *entityRequester = data->entityRequester;

  if(entityRequester) {
    WindowToFront(entityRequester->window->intuitionWindow);
  } else {
    entityRequester = newEntityRequester();
    if(entityRequester) {
      /* attachEntityRequesterToEntityBrowser(data, entityRequester); */
      /* TODO: fix me */
      /* addWindowToSet(entityRequester->window); */
    }
  }
}

static void closeEntityBrowser(FrameworkWindow *entityBrowser) {
  EntityBrowserData *data = entityBrowser->data;
  free(data->title);
  freeEntityLabels(data);
  freeTagLabels(data);
  free(data);
}

static WindowKind entityBrowserWindowKind = {
  {
    40, 40, ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|ACTIVATE,
    NULL,
    NULL,
    "Entities",
    NULL,
    NULL,
    ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    ENTITY_BROWSER_WIDTH, ENTITY_BROWSER_HEIGHT,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    &closeEntityBrowser
};

static ListViewSpec entityListSpec = {
  ENTITY_LIST_LEFT, ENTITY_LIST_TOP,
  ENTITY_LIST_WIDTH, ENTITY_LIST_HEIGHT,
  NULL
};

static ButtonSpec addEntitySpec = {
  ADD_ENTITY_LEFT, ADD_ENTITY_TOP,
  ADD_ENTITY_WIDTH, ADD_ENTITY_HEIGHT,
  "Add Entity...",
  TEXT_INSIDE,
  ENABLED,
  onAddEntityClick
};

static ButtonSpec removeEntitySpec = {
  REMOVE_ENTITY_LEFT, REMOVE_ENTITY_TOP,
  REMOVE_ENTITY_WIDTH, REMOVE_ENTITY_HEIGHT,
  "Remove Entity",
  TEXT_INSIDE,
  DISABLED,
  onRemoveEntityClick
};

static IntegerSpec entityRowSpec = {
  ENTITY_ROW_LEFT,  ENTITY_ROW_TOP,
  ENTITY_ROW_WIDTH, ENTITY_ROW_HEIGHT,
  "Row",
  TEXT_ON_LEFT,
  1,
  DISABLED,
  onEntityRowEntry
};

static IntegerSpec entityColSpec = {
  ENTITY_COL_LEFT,  ENTITY_COL_TOP,
  ENTITY_COL_WIDTH, ENTITY_COL_HEIGHT,
  "Column",
  TEXT_ON_LEFT,
  1,
  DISABLED,
  onEntityColEntry
};

static IntegerSpec VRAMSlotSpec = {
  VRAM_SLOT_LEFT, VRAM_SLOT_TOP,
  VRAM_SLOT_WIDTH, VRAM_SLOT_HEIGHT,
  "VRAM Slot",
  TEXT_ON_LEFT,
  3,
  DISABLED,
  onVRAMSlotEntry
};

static TextSpec thisEntitySpec = {
  THIS_ENTITY_LEFT, THIS_ENTITY_TOP,
  THIS_ENTITY_WIDTH, THIS_ENTITY_HEIGHT,
  "Entity",
  TEXT_ON_LEFT,
  "N/A",
  BORDERED
};

static ButtonSpec chooseEntitySpec = {
  CHOOSE_ENTITY_LEFT, CHOOSE_ENTITY_TOP,
  CHOOSE_ENTITY_WIDTH, CHOOSE_ENTITY_HEIGHT,
  "...",
  TEXT_INSIDE,
  DISABLED,
  onChooseEntityClick
};

static ListViewSpec tagListSpec = {
  TAG_LIST_LEFT, TAG_LIST_TOP,
  TAG_LIST_WIDTH, TAG_LIST_HEIGHT,
  NULL
};

static ButtonSpec addTagSpec = {
  ADD_TAG_LEFT, ADD_TAG_TOP,
  ADD_TAG_WIDTH, ADD_TAG_HEIGHT,
  "Add Tag",
  TEXT_INSIDE,
  DISABLED,
  onAddTagClick
};

static ButtonSpec deleteTagSpec = {
  DELETE_TAG_LEFT, DELETE_TAG_TOP,
  DELETE_TAG_WIDTH, DELETE_TAG_HEIGHT,
  "Delete Tag",
  TEXT_INSIDE,
  DISABLED,
  onDeleteTagClick
};

static StringSpec tagAliasSpec = {
  TAG_ALIAS_LEFT, TAG_ALIAS_TOP,
  TAG_ALIAS_WIDTH, TAG_ALIAS_HEIGHT,
  "Alias",
  TEXT_ON_LEFT,
  DISABLED,
  onTagAliasEntry
};

static IntegerSpec tagIdSpec = {
  TAG_ID_LEFT, TAG_ID_TOP,
  TAG_ID_WIDTH, TAG_ID_HEIGHT,
  "ID",
  TEXT_ON_LEFT,
  3,
  DISABLED,
  onTagIdEntry
};

static IntegerSpec tagValueSpec = {
  TAG_VALUE_LEFT, TAG_VALUE_TOP,
  TAG_VALUE_WIDTH, TAG_VALUE_HEIGHT,
  "Value",
  TEXT_ON_LEFT,
  3,
  DISABLED,
  onTagValueEntry
};

FrameworkWindow *newEntityBrowser(FrameworkWindow *parent, const Map *map, int mapNum) {
  EntityBrowserData *data = malloc(sizeof(EntityBrowserData));
  struct Gadget *gadgets;
  FrameworkWindow *entityBrowser;

  if(!data) {
    fprintf(stderr, "newEntityBrowser: couldn't allocate entity browser data\n");
    goto error;
  }

  data->entityRequester = NULL;
  data->selectedEntity = 0;
  data->selectedTag = 0;

  data->title = malloc(32);
  if(!data->title) {
    fprintf(stderr, "newEntityBrowser: couldn't allocate title\n");
    goto error_freeData;
  }
  if(mapNum) {
    sprintf(data->title, "Entities (Map %d)", mapNum - 1);
  } else {
    strcpy(data->title, "Entities");
  }
  entityBrowserWindowKind.newWindow.Title = data->title;

  addEntitySpec.state = map->entityCnt < MAX_ENTITIES_PER_MAP ? ENABLED : DISABLED;
  /* TODO: this is probably broken huh. */
  entityListSpec.labels = &data->entityLabels;
  gadgets = buildGadgets(
    makeButtonGadget(&addEntitySpec),    &data->addEntityGadget,
    makeButtonGadget(&removeEntitySpec), &data->removeEntityGadget,
    makeButtonGadget(&chooseEntitySpec), &data->chooseEntityGadget,
    makeButtonGadget(&addTagSpec),       &data->addTagGadget,
    makeButtonGadget(&deleteTagSpec),    &data->deleteTagGadget,
    makeTextGadget(&thisEntitySpec),     &data->thisEntityGadget, 
    makeStringGadget(&tagAliasSpec),     &data->tagAliasGadget,
    makeIntegerGadget(&entityRowSpec),   &data->rowGadget,
    makeIntegerGadget(&entityColSpec),   &data->colGadget,
    makeIntegerGadget(&VRAMSlotSpec),    &data->VRAMSlotGadget,
    makeIntegerGadget(&tagIdSpec),       &data->tagIdGadget,
    makeIntegerGadget(&tagValueSpec),    &data->tagValueGadget,
    makeListViewGadget(&entityListSpec), &data->entityListGadget,
    makeListViewGadget(&tagListSpec),    &data->tagListGadget,
    NULL);

  if(!gadgets) {
    fprintf(stderr, "newMapEditor: failed to create gadgets\n");
    goto error_freeTitle;
  }

  entityBrowser = openChildWindow(parent, &entityBrowserWindowKind, gadgets);
  if(!entityBrowser) {
    fprintf(stderr, "newEntityBrowser: couldn't open window\n");
    goto error_freeGadgets;
  }

  entityBrowser->data = data;

  entityBrowserRefreshEntities(entityBrowser);

  return entityBrowser;

error_freeGadgets:
  /* TODO: we free this twice on window creation failure! */
  FreeGadgets(gadgets);
error_freeTitle:
  free(data->title);
error_freeData:
  free(data);
error:
  return NULL;
}
