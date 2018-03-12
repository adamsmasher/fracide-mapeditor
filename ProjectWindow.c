#include "ProjectWindow.h"

#include <libraries/asl.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "framework/menubuild.h"
#include "framework/screen.h"

#include "easystructs.h"
#include "entitiesmenu.h"
#include "MapEditor.h"
#include "mapmenu.h"
#include "musicmenu.h"
#include "Project.h"
#include "projectmenu.h"
#include "ProjectWindowData.h"

static MenuSpec mainMenuSpec[] = {
  { "Project",  &projectMenuSpec  },
  { "Maps",     &mapMenuSpec      },
  { "Entities", &entitiesMenuSpec },
  { "Music",    &musicMenuSpec    },
  END_MENUS
};

#define REVERT_PROJECT_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static BOOL unsavedProjectAlert(FrameworkWindow *projectWindow) {
  int response;

  response = EasyRequest(
    projectWindow->intuitionWindow,
    &unsavedProjectAlertEasyStruct,
    NULL);

  switch(response) {
    case 0: return FALSE;
    case 1: return saveProject(projectWindow);
    case 2: return TRUE;
    default:
      fprintf(stderr, "unsavedProjectAlert: unknown response %d\n", response);
      goto error;
  }

error:
  return FALSE;
}

static BOOL ensureProjectSaved(FrameworkWindow *projectWindow) {
  return (BOOL)(projectDataIsSaved(projectWindow->data) || unsavedProjectAlert(projectWindow));
}

static BOOL ensureMapEditorsSaved(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditorWindow(i)) {
      if(!ensureMapEditorSaved(i->data)) {
        return FALSE;
      }
    }
    i = i->next;
  }
}

static void onClose(FrameworkWindow *projectWindow) {
  freeProjectData(projectWindow->data);
}

static BOOL ensureEverythingSaved(FrameworkWindow *projectWindow) {
  return (BOOL)(ensureMapEditorsSaved(projectWindow) && ensureProjectSaved(projectWindow));
}

static WindowKind projectWindowKind = {
  {
    0,0, -1, -1,
    0xFF,0xFF,
    MENUPICK,
    BORDERLESS|BACKDROP,
    NULL,
    NULL,
    "Project",
    NULL,
    NULL,
    -1, -1,
    0xFFFF,0xFFFF,
    CUSTOMSCREEN
  },
  (MenuSpec*)        mainMenuSpec,
  (RefreshFunction)  NULL,
  (CanCloseFunction) ensureEverythingSaved,
  (CloseFunction)    onClose
};

static void makeWindowFullScreen(void) {
  struct NewWindow *newWindow = &projectWindowKind.newWindow;
  newWindow->MinWidth  = newWindow->Width  = getScreenWidth();
  newWindow->MinHeight = newWindow->Height = getScreenHeight();
}

FrameworkWindow *openProjectWindow(void) {
  FrameworkWindow *projectWindow;
  ProjectWindowData *data;

  makeWindowFullScreen();

  data = createProjectData();
  if(!data) {
    fprintf(stderr, "openProjectWindow: failed to allocate data\n");
    goto error;
  }

  projectWindow = openWindowOnGlobalScreen(&projectWindowKind);
  if(!projectWindow) {
    fprintf(stderr, "openProjectWindow: failed to open window!\n");
    goto error_freeData;
  }

  projectWindow->data = data;

  ActivateWindow(projectWindow->intuitionWindow);

  return projectWindow;

error_freeData:
  freeProjectData(data);
error:
  return NULL;
}

static void setProjectFilename(FrameworkWindow *projectWindow, char *filename) {
  setProjectDataFilename(projectWindow->data, filename);
  OnMenu(projectWindow->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
}

static void clearProjectFilename(FrameworkWindow *projectWindow) {
  clearProjectDataFilename(projectWindow->data);
  OffMenu(projectWindow->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
}

static BOOL saveProjectToAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
  size_t bufferLen = strlen(dir) + strlen(file) + 2;
  char *buffer = malloc(bufferLen);

  if(!buffer) {
    fprintf(
      stderr,
      "saveProjectToAsl: failed to allocate buffer "
      "(dir: %s) (file: %s)\n",
      dir  ? dir  : "NULL",
      file ? file : "NULL");
    goto error;
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
    goto error_freeBuffer;
  }

  if(!projectDataSaveProjectToFile(projectWindow->data, buffer)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &projectSaveFailEasyStruct,
      NULL,
      buffer);
    goto error_freeBuffer;
  }
  setProjectFilename(projectWindow, buffer);

freeBuffer:
  free(buffer);
done:
  return TRUE;

error_freeBuffer:
  free(buffer);
error:
  return FALSE;
}

BOOL saveProjectAs(FrameworkWindow *projectWindow) {
  BOOL result;
  struct FileRequester *request; 

  request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Save Project As",
    ASL_Window, projectWindow->intuitionWindow,
    ASL_FuncFlags, FILF_SAVE,
    TAG_END);
  if(!request) {
    fprintf(stderr, "saveProjectAs: couldn't allocate asl request tags\n");
    goto error;
  }

  result = AslRequest(request, NULL);
  if(result) {
    result = saveProjectToAsl(projectWindow, request->rf_Dir, request->rf_File);
  }

  FreeAslRequest(request);
done:
  return result;
error:
  return FALSE;
}

BOOL saveProject(FrameworkWindow *projectWindow) {
  ProjectWindowData *data = projectWindow->data;
  char *filename = projectDataGetFilename(data);

  if(filename) {
    if(!projectDataSaveProjectToFile(data, filename)) {
      EasyRequest(
        projectWindow->intuitionWindow,
        &projectSaveFailEasyStruct,
        NULL,
        filename);
      goto error;
    }
  } else {
    return saveProjectAs(projectWindow);
  }

done:
  return TRUE;

error:
  return FALSE;
}

static BOOL loadTilesetPackageFromFile(FrameworkWindow *projectWindow, char *filename) {
  if(!projectDataLoadTilesetPackage(projectWindow->data, filename)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &tilesetPackageLoadFailEasyStruct,
      NULL,
      filename);
    goto error;
  }

done:
  return TRUE;

error:
  return FALSE;
}

static void openProjectFromFile(FrameworkWindow *projectWindow, char *filename) {
  switch(projectDataLoadProjectFromFile(projectWindow->data, filename)) {
    case PROJECT_LOAD_OK:
      break;
    case PROJECT_LOAD_OK_TILESET_ERROR:
      EasyRequest(
        projectWindow->intuitionWindow,
        &tilesetPackageLoadFailEasyStruct,
        NULL,
        projectDataGetTilesetPath(projectWindow->data));
      break;
    case PROJECT_LOAD_ERROR:
      EasyRequest(
        projectWindow->intuitionWindow,
        &projectLoadFailEasyStruct,
        NULL,
        filename);
      goto error;
  }

  setProjectFilename(projectWindow, filename);

done:
    return;

error:
    return;
}

static void openProjectFromAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
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

  openProjectFromFile(projectWindow, buffer);

freeBuffer:
    free(buffer);
done:
    return;
}

void newProject(FrameworkWindow *projectWindow) {
  if(ensureEverythingSaved(projectWindow)) {
    projectDataInitProject(projectWindow->data);
    clearProjectFilename(projectWindow);
  }
}

void openProject(FrameworkWindow *projectWindow) {
  struct FileRequester *request;

  if(!ensureEverythingSaved(projectWindow)) {
    goto done;
  }

  request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Open Project",
    ASL_Window, projectWindow->intuitionWindow,
    TAG_END);
  if(!request) {
    goto done;
  }

  if(AslRequest(request, NULL)) {
    openProjectFromAsl(projectWindow, request->rf_Dir, request->rf_File);
  }

  FreeAslRequest(request);
done:
  return;
error:
  return;
}

static int confirmRevertProject(FrameworkWindow *projectWindow) {
  return EasyRequest(
    projectWindow->intuitionWindow,
    &confirmRevertProjectEasyStruct,
    NULL);
}

void revertProject(FrameworkWindow *projectWindow) {
  if(!confirmRevertProject(projectWindow)) {
    goto done;
  }

  openProjectFromFile(projectWindow, projectDataGetFilename(projectWindow->data));

done:
  return;
}

static void updateAllTileDisplays(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditorWindow(i)) {
      mapEditorUpdateTileDisplays(i);
    }
    i = i->next;
  }
}

/* TODO: maybe push the UI logic out */
/* TODO: best still, add Asl handling to the framework */
static BOOL loadTilesetPackageFromAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
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

  return loadTilesetPackageFromFile(projectWindow, buffer);

error:
  return FALSE;
}

void selectTilesetPackage(FrameworkWindow *projectWindow) {
  struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Select Tileset Package",
    ASL_Window, projectWindow->intuitionWindow,
    TAG_END);
  if(!request) {
    fprintf(stderr, "selectTilesetPackage: failed to allocate requester\n");
    goto error;
  }

  if(AslRequest(request, NULL)) {
    if(loadTilesetPackageFromAsl(projectWindow, request->rf_Dir, request->rf_File)) {
      updateAllTileDisplays(projectWindow);
    }
  }

  FreeAslRequest(request);

done:
  return;
error:
  return;
}

void quit(FrameworkWindow *projectWindow) {
  tryToCloseWindow(projectWindow);
}

/* TODO: this is weird */
/* TODO: just pass in data? */
/* static BOOL currentProjectSaveNewMap(FrameworkWindow *projectWindow, Map *map, int mapNum) {
  ProjectWindowData *data = projectWindow->data;

  Map *mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "currentProjectSaveNewMap: couldn't allocate map copy\n");
    return FALSE;
  }
  data->project.mapCnt++;
  data->project.maps[mapNum] = mapCopy;
  return TRUE;
} */

/* TODO: just take data? */
/* static void currentProjectOverwriteMap(FrameworkWindow *projectWindow, Map *map, int mapNum) {
  ProjectWindowData *data = projectWindow->data;
  overwriteMap(map, data->project.maps[mapNum]);
} */

/* TODO: just pass in data? */
/* TODO: this is weird */
/* static void updateCurrentProjectMapName(FrameworkWindow *projectWindow, int mapNum, Map *map) {
  ProjectWindowData *data = projectWindow->data;
  updateProjectMapName(&data->project, mapNum, map);
} */

static int confirmCreateMap(FrameworkWindow *projectWindow, int mapNum) {
  return EasyRequest(
    projectWindow->intuitionWindow,
    &confirmCreateMapEasyStruct,
    NULL,
    mapNum);
}

/* TODO: I kind of feel that this belongs in MapEditor, maybe? */
BOOL openMapNum(FrameworkWindow *projectWindow, int mapNum) {
  FrameworkWindow *mapEditor;

  if(!projectDataHasMap(projectWindow->data, mapNum)) {
    if(!confirmCreateMap(projectWindow, mapNum)) {
      goto error;
    }

    if(!projectDataCreateMap(projectWindow->data, mapNum)) {
      fprintf(stderr, "openMapNum: failed to create map\n");
      goto error;
    }
  }

  mapEditor = newMapEditorWithMap(projectDataGetMap(projectWindow->data, mapNum), mapNum);
  if(!mapEditor) {
    fprintf(stderr, "openMapNum: failed to create new map editor\n");
    goto error;
  }

  /* TODO: fix us */
  /* addToMapEditorSet(mapEditor); */
  /* addWindowToSet(mapEditor->window); */
  enableMapRevert(mapEditor);

done:
  return TRUE;

error:
  return FALSE;
}

static FrameworkWindow *findMapEditor(FrameworkWindow *projectWindow, int mapNum) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditorWindow(i)) {
      MapEditorData *data = i->data;
      if(data->mapNum - 1 == mapNum) {
        return i;
      }
    }
    i = i->next;
  }
  return NULL;
}

void newMap(FrameworkWindow *projectWindow) {
  FrameworkWindow *mapEditor = newMapEditorNewMap();
  if(!mapEditor) {
    fprintf(stderr, "newMap: failed to create mapEditor\n");
    return;
  }
  /* TODO: fix us */
  /* addToMapEditorSet(mapEditor);
    addWindowToSet(mapEditor->window);*/
}

void openMap(FrameworkWindow *projectWindow) {
  FrameworkWindow *mapEditor;

  int selected = openMapRequester(projectWindow);
  if(!selected) {
    return;
  }
  selected--;

  mapEditor = findMapEditor(projectWindow, selected);
  if(mapEditor) {
    WindowToFront(mapEditor->intuitionWindow);
  } else {
    openMapNum(projectWindow, selected);
  }
}

void refreshAllSongDisplays(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditorWindow(i)) {
      /* TODO: make this a function on map requesters */
      MapEditorData *data = i->data;
      if(data->songRequester) {
        GT_RefreshWindow(data->songRequester->window->intuitionWindow, NULL);
      }
      mapEditorRefreshSong(i);
    }
    i = i->next;
  }
}

void refreshAllEntityBrowsers(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditorWindow(i)) {
      /* TODO: make this a function on map requesters */
      MapEditorData *data = i->data;
      if(data->entityBrowser) {
        GT_RefreshWindow(data->entityBrowser->window->intuitionWindow, NULL);
      }
    }
    i = i->next;
  }
}
