#include "SongRequester.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <proto/intuition.h>

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <graphics/gfx.h>
#include <proto/graphics.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/font.h"
#include "framework/gadgets.h"
#include "framework/menubuild.h"
#include "framework/screen.h"
#include "framework/window.h"

#include "MapEditorData.h"
#include "NumberedList.h"
#include "Project.h"
#include "ProjectWindow.h"
#include "ProjectWindowData.h"

#define SONG_REQUESTER_WIDTH      200
#define SONG_REQUESTER_HEIGHT     336
#define SONG_REQUESTER_MIN_HEIGHT 48

#define SONG_LIST_WIDTH_DELTA  35
#define SONG_LIST_HEIGHT_DELTA 38
#define SONG_LIST_TOP          20
#define SONG_LIST_LEFT         10

#define SONG_NAME_WIDTH_DELTA   SONG_LIST_WIDTH_DELTA
#define SONG_NAME_HEIGHT        12
#define SONG_NAME_BOTTOM_OFFSET 20
#define SONG_NAME_LEFT          SONG_LIST_LEFT

typedef struct SongRequesterGadgets_tag {
  struct Gadget *songNameGadget;
  struct Gadget *songListGadget;
} SongRequesterGadgets;

typedef enum Editable_tag {
  NON_EDITABLE,
  EDITABLE
} Editable;

typedef struct SongRequesterData_tag {
  UWORD selected;
  Editable editable;
  struct List *songNames;
  char *title;
} SongRequesterData;

static void songRequesterOnSelectSong(FrameworkWindow *songRequester, UWORD selected) {
  SongRequesterData *data = songRequester->data;
  SongRequesterGadgets *gadgets = songRequester->gadgets->data;

  data->selected = selected + 1;

  if(data->editable) {
    ProjectWindowData *projectData = songRequester->parent->data;
    const char *songName = projectDataGetSongName(projectData, selected);

    GT_SetGadgetAttrs(gadgets->songNameGadget, songRequester->intuitionWindow, NULL,
      GTST_String, songName,
      GA_Disabled, FALSE,
      TAG_END);
  } else {
    FrameworkWindow *mapEditor = songRequester->parent;
    mapEditorDataSetSong(mapEditor->data, selected);  
  }
}

static void songRequesterOnNameEntry(FrameworkWindow *songRequester) {
  SongRequesterData *data = songRequester->data;
  SongRequesterGadgets *gadgets = songRequester->gadgets->data;
  FrameworkWindow *projectWindow = songRequester->parent;
  ProjectWindowData *projectData = projectWindow->data;

  UWORD selected = data->selected - 1;

  char *name = ((struct StringInfo*)gadgets->songNameGadget->SpecialInfo)->Buffer;

  projectDataUpdateSongName(projectData, selected, name);
  numberedListSetItem(data->songNames, selected, name);

  GT_RefreshWindow(songRequester->intuitionWindow, NULL);
  projectWindowRefreshAllSongDisplays(projectWindow);
}

static ListViewSpec songListSpec = {
  SONG_LIST_LEFT,  SONG_LIST_TOP,
  SONG_REQUESTER_WIDTH - SONG_LIST_WIDTH_DELTA,
  SONG_REQUESTER_HEIGHT - SONG_LIST_HEIGHT_DELTA,
  (struct List*) NULL, /* fill this out before creation */
  (struct Gadget**)NULL, /* fill this out before creation */
  songRequesterOnSelectSong
};

static StringSpec songNameSpec = {
  SONG_NAME_LEFT,  SONG_REQUESTER_HEIGHT - SONG_NAME_BOTTOM_OFFSET,
  SONG_REQUESTER_WIDTH - SONG_NAME_WIDTH_DELTA, SONG_NAME_HEIGHT,
  64,
  (char*)NULL,
  TEXT_ON_LEFT,
  DISABLED,
  songRequesterOnNameEntry
};

static WindowGadgets *createSongRequesterGadgets(int width, int height, SongRequesterData *data) {
  SongRequesterGadgets *gadgetData;
  WindowGadgets *gadgets;

  gadgets = malloc(sizeof(WindowGadgets));
  if(!gadgets) {
    fprintf(stderr, "createSongRequesterGadgets: couldn't allocate window gadgets\n");
    goto error;
  }

  gadgetData = malloc(sizeof(SongRequesterGadgets));
  if(!gadgetData) {
    fprintf(stderr, "createSongRequesterGadgets: couldn't allocate data\n");
    goto error_freeWindowGadgets;
  }

  songListSpec.height = height - SONG_LIST_HEIGHT_DELTA;
  songListSpec.width  = width  - SONG_LIST_WIDTH_DELTA;
  songListSpec.labels = data->songNames;

  if(data->editable) {
    songNameSpec.top   = height - SONG_NAME_BOTTOM_OFFSET;
    songNameSpec.width = width  - SONG_NAME_WIDTH_DELTA;

    gadgets->glist = buildGadgets(
      makeStringGadget(&songNameSpec), &gadgetData->songNameGadget,
      makeListViewGadget(&songListSpec), &gadgetData->songListGadget,
      NULL);
  } else {
    gadgets->glist = buildGadgets(
      makeListViewGadget(&songListSpec), &gadgetData->songListGadget,
      NULL);
  }
  if(!gadgets->glist) {
    fprintf(stderr, "createSongRequesterGadgets: failed to create gadgets\n");
    goto error_freeSongRequesterGadgets;
  }

  gadgets->data = gadgetData;

  return gadgets;

error_freeSongRequesterGadgets:
  free(gadgetData);
error_freeWindowGadgets:
  free(gadgets);
error:
  return NULL;
}

static void freeSongRequesterGadgets(WindowGadgets *gadgets) {
  FreeGadgets(gadgets->glist);
  free(gadgets->data);
  free(gadgets);
}

static void closeSongRequester(FrameworkWindow *songRequester) {
  SongRequesterData *data = songRequester->data;
  freeNumberedList(data->songNames);
  free(data->title);
  free(data);
}

static WindowKind songRequesterWindowKind = {
  {
    40, 40, SONG_REQUESTER_WIDTH, SONG_REQUESTER_HEIGHT,
    0xFF, 0xFF,
    CLOSEWINDOW|REFRESHWINDOW|GADGETUP|LISTVIEWIDCMP|NEWSIZE,
    WINDOWCLOSE|WINDOWDEPTH|WINDOWDRAG|WINDOWSIZING|ACTIVATE,
    NULL,
    NULL,
    "Edit Song Names",
    NULL,
    NULL,
    SONG_REQUESTER_WIDTH, SONG_REQUESTER_MIN_HEIGHT,
    0xFFFF, 0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        NULL,
  (GadgetBuilder)    createSongRequesterGadgets,
  (GadgetFreer)      freeSongRequesterGadgets,
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL,
  (CloseFunction)    NULL,
  (ClickFunction)    NULL,
};

static FrameworkWindow *newGenericSongRequester(FrameworkWindow *parent, char *title, ProjectWindowData *projectData, Editable editable) {
  struct List *songNameList;
  SongRequesterData *data;
  FrameworkWindow *songRequester;

  songNameList = newNumberedList(projectDataGetSongName, projectData, MAX_SONGS_IN_PROJECT);
  if(!songNameList) {
    fprintf(stderr, "newGenericSongRequester: couldn't make song name list\n");
    goto error;
  }

  data = malloc(sizeof(SongRequesterData));
  if(!data) {
    fprintf(stderr, "newGenericSongRequester: couldn't allocate data\n");
    goto error_freeNameList;
  }

  data->editable = editable;
  data->selected = 0;
  data->songNames = songNameList;

  data->title = strdup(title);
  if(!data->title) {
    fprintf(stderr, "newGenericSongRequester: couldn't copy title\n");
    goto error_freeData;
  }

  songRequesterWindowKind.newWindow.Title = data->title;

  songRequester = openChildWindow(parent, &songRequesterWindowKind, data);
  if(!songRequester) {
    fprintf(stderr, "newGenericSongRequester: couldn't open window\n");
    goto error_freeTitle;
  }

  return songRequester;

error_freeTitle:
  free(data->title);
error_freeData:
  free(data);
error_freeNameList:
  freeNumberedList(songNameList);
error:
  return NULL;
}

FrameworkWindow *newSongRequester(FrameworkWindow *mapEditor) {
  FrameworkWindow *projectWindow = mapEditor->parent;
  MapEditorData *mapEditorData = mapEditor->data;
  char title[] = "Change Soundtrack for Map XXX";

  if(mapEditorDataHasMapNum(mapEditorData)) {
    sprintf(title, "Change Soundtrack for Map %d", mapEditorDataGetMapNum(mapEditorData));
  } else {
    sprintf(title, "Change Soundtrack");
  }

  return newGenericSongRequester(mapEditor, title, projectWindow->data, NON_EDITABLE);
}

FrameworkWindow *newSongNamesEditor(FrameworkWindow *projectWindow) {
  return newGenericSongRequester(projectWindow, "Edit Song Names", projectWindow->data, EDITABLE);
}

BOOL isSongRequester(FrameworkWindow *window) {
  return (BOOL)(window->kind == &songRequesterWindowKind);
}

BOOL isSongNamesEditor(FrameworkWindow *window) {
  if(isSongRequester(window)) {
    SongRequesterData *data = window->data;
    return (BOOL)(data->editable == EDITABLE);
  }
  return FALSE;
}

void songRequesterRefresh(FrameworkWindow *songRequester) {
  ProjectWindowData *projectData;
  SongRequesterData *data = songRequester->data;
  SongRequesterGadgets *gadgets = songRequester->gadgets->data;

  if(data->editable) {
    projectData = songRequester->parent->data;
  } else {
    projectData = songRequester->parent->parent->data;
  }

  GT_SetGadgetAttrs(gadgets->songListGadget, songRequester->intuitionWindow, NULL,
    GTLV_Labels, ~0,
    TAG_END);

  freeNumberedList(data->songNames);
  data->songNames = newNumberedList(projectDataGetSongName, projectData, MAX_SONGS_IN_PROJECT);
  if(!data->songNames) {
    fprintf(stderr, "songRequesterRefresh: couldn't make song name list\n");
    goto error;
  }

  GT_SetGadgetAttrs(gadgets->songListGadget, songRequester->intuitionWindow, NULL,
    GTLV_Labels, data->songNames,
    TAG_END);

  GT_RefreshWindow(songRequester->intuitionWindow, NULL);

  return;
error:
  return;
}
