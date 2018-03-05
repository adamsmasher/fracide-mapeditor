/* TODO: make all public functions take a this argument... */

#include "ProjectWindow.h"

#include <libraries/asl.h>
#include <proto/asl.h>
#include <proto/dos.h>
#include <proto/intuition.h>

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
#include "projectmenu.h"

static MenuSpec mainMenuSpec[] = {
  { "Project",  &projectMenuSpec  },
  { "Maps",     &mapMenuSpec      },
  { "Entities", &entitiesMenuSpec },
  { "Music",    &musicMenuSpec    },
  END_MENUS
};

#define REVERT_PROJECT_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static FrameworkWindow *projectWindow = NULL;
static struct Menu     *menu          = NULL;

static void onClose(FrameworkWindow *window) {
  ProjectWindowData *data = window->data;
  freeProject(&data->project);
  free(data);
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
  (MenuSpec*)        NULL, /* set me later */
  (RefreshFunction)  NULL,
  (CanCloseFunction) NULL, /* TODO: check for saved/unsaved here */
  (CloseFunction)    onClose
};

FrameworkWindow *getProjectWindow(void) {
  return projectWindow;
}

static void makeWindowFullScreen(void) {
  struct NewWindow *newWindow = &projectWindowKind.newWindow;
  newWindow->MinWidth  = newWindow->Width  = getScreenWidth();
  newWindow->MinHeight = newWindow->Height = getScreenHeight();
}

BOOL openProjectWindow(void) {
  ProjectWindowData *data;

  if(projectWindow) {
    fprintf(stderr, "openProjectWindow: cannot be called when project window already exists\n");
    goto error;
  }

  projectWindowKind.menuSpec = mainMenuSpec;

  makeWindowFullScreen();

  data = malloc(sizeof(ProjectWindowData));
  if(!data) {
    fprintf(stderr, "openProjectWindow: failed to allocate data\n");
    goto error;
  }

  projectWindow = openWindowOnGlobalScreen(&projectWindowKind);
  if(!projectWindow) {
    fprintf(stderr, "openProjectWindow: failed to open window!\n");
    goto error_freeData;
  }

  initProject(&data->project);
  data->projectSaved = TRUE;
  data->projectFilename[0] = '\0';

  projectWindow->data = data;

  ActivateWindow(projectWindow->intuitionWindow);

  return TRUE;

error_freeData:
  free(data);
error:
  return FALSE;
}

void closeProjectWindow(void) {
  if(!projectWindow) {
    fprintf(stderr, "closeProjectWindow: projectWindow not yet opened!\n");
    return;
  }

  closeWindow(projectWindow);

  projectWindow = NULL;
}

static void setProjectFilename(FrameworkWindow *projectWindow, char *filename) {
  ProjectWindowData *data = projectWindow->data;

  /* TODO: honestly these OnMenu/OffMenu things could probably be made into functions elsewhere */
  if(filename) {
    strcpy(data->projectFilename, filename);
    OnMenu(projectWindow->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
  } else {
    data->projectFilename[0] = '\0';
    OffMenu(projectWindow->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
  }
}

static BOOL saveProjectToAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
  ProjectWindowData *data = projectWindow->data;
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

  if(!saveProjectToFile(&data->project, buffer)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &projectSaveFailEasyStruct,
      NULL,
      buffer);
    goto error_freeBuffer;
  }
  setProjectFilename(projectWindow, buffer);

  data->projectSaved = TRUE;

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

  if(data->projectFilename[0]) {
    if(!saveProjectToFile(&data->project, data->projectFilename)) {
      EasyRequest(
        projectWindow->intuitionWindow,
        &projectSaveFailEasyStruct,
        NULL,
        data->projectFilename);
      goto error;
    } else {
      return TRUE;
    }
  } else {
    return saveProjectAs(projectWindow);
  }

error:
  return FALSE;
}

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
  ProjectWindowData *data = projectWindow->data;
  return (BOOL)(data->projectSaved || unsavedProjectAlert(projectWindow));
}

/* TODO: maybe even just make this take data? */
/* Note that this doesn't INIT the project, in case you're going
   to load a project in, for example */
static void clearProject(FrameworkWindow *projectWindow) {
  ProjectWindowData *data = projectWindow->data;

  closeAllMapEditors();
  freeTilesetPackage(data->tilesetPackage);
  data->tilesetPackage = NULL;
  freeProject(&data->project);
  data->projectSaved = TRUE;
}

static void openProjectFromFile(char *file) {
  Project *myNewProject;
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "openProjectFromFile: couldn't get project window\n");
    goto error;
  }

  data = window->data;

  myNewProject = malloc(sizeof(Project));
  if(!myNewProject) {
    fprintf(stderr, "openProjectFromFile: failed to allocate project\n");
    goto error;
  }

  if(!loadProjectFromFile(file, myNewProject)) {
    EasyRequest(
      window->intuitionWindow,
      &projectLoadFailEasyStruct,
      NULL,
      file);
    goto freeProject;
  }

  clearProject(projectWindow);
  copyProject(myNewProject, &data->project);
  setProjectFilename(projectWindow, file);

  if(*data->project.tilesetPackagePath && !loadTilesetPackageFromFile(data->project.tilesetPackagePath)) {
    EasyRequest(
      window->intuitionWindow,
      &tilesetPackageLoadFailEasyStruct,
      NULL,
      data->project.tilesetPackagePath);

      /* because the tileset will now be empty, we've changed from the
         saved version */
      data->projectSaved = FALSE;
      goto error_freeProject;
    }

freeProject:
    free(myNewProject);
done:
    return;

error_freeProject:
    free(myNewProject);
error:
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

static BOOL ensureEverythingSaved(FrameworkWindow *projectWindow) {
  return (BOOL)(ensureMapEditorsSaved() && ensureProjectSaved(projectWindow));
}

void newProject(FrameworkWindow *projectWindow) {
  ProjectWindowData *data = projectWindow->data;

  if(ensureEverythingSaved(projectWindow)) {
    clearProject(projectWindow);
    initProject(&data->project);
    setProjectFilename(projectWindow, NULL);
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
    openProjectFromAsl(request->rf_Dir, request->rf_File);
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

static char *getProjectFilename(ProjectWindowData *data) {
  return data->projectFilename;
}

void revertProject(FrameworkWindow *projectWindow) {
  if(!confirmRevertProject(projectWindow)) {
    goto done;
  }

  openProjectFromFile(getProjectFilename(projectWindow->data));

done:
  return;
}

static void updateAllTileDisplays(void) {
/*  MapEditor *i = mapEditorSetFirst();
  while(i) {
    if(i->tilesetRequester) {
      refreshTilesetRequesterList(i->tilesetRequester);
    }
    mapEditorRefreshTileset(i);
    i = i->next;
  } */
/* TODO: fix me */
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
    if(loadTilesetPackageFromAsl(request->rf_Dir, request->rf_File)) {
      updateAllTileDisplays();
    }
  }

  FreeAslRequest(request);

done:
  return;
error:
  return;
}

void quit(FrameworkWindow *projectWindow) {
  if(ensureEverythingSaved(projectWindow)) {
    stopRunning();
  }
}

/* TODO: just send in data? */
static void setTilesetPackagePath(FrameworkWindow *projectWindow, char *file) {
  ProjectWindowData *data = projectWindow->data;

  strcpy(data->project.tilesetPackagePath, file);
  data->projectSaved = FALSE;
}

/* TODO: just pass in data? */
static BOOL createMap(FrameworkWindow *projectWindow, int mapNum) {
  ProjectWindowData *data = projectWindow->data;

  data->project.maps[mapNum] = allocMap();
  if(!data->project.maps[mapNum]) {
    fprintf(stderr, "createMap: failed to allocate new map\n");
    return FALSE;
  }
  data->project.mapCnt++;
  data->projectSaved = FALSE;
  return TRUE;
}

/* TODO: this is weird */
/* TODO: just pass in data? */
static BOOL currentProjectSaveNewMap(FrameworkWindow *projectWindow, Map *map, int mapNum) {
  ProjectWindowData *data = projectWindow->data;

  Map *mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "currentProjectSaveNewMap: couldn't allocate map copy\n");
    return FALSE;
  }
  data->project.mapCnt++;
  data->project.maps[mapNum] = mapCopy;
  return TRUE;
}

/* TODO: just take data? */
static void currentProjectOverwriteMap(FrameworkWindow *projectWindow, Map *map, int mapNum) {
  ProjectWindowData *data = projectWindow->data;
  overwriteMap(map, data->project.maps[mapNum]);
}

/* TODO: just take data? */
static BOOL currentProjectHasMap(FrameworkWindow *projectWindow, int mapNum) {
  ProjectWindowData *data = projectWindow->data;
  return (BOOL)(data->project.maps[mapNum] ? TRUE : FALSE);
}

/* TODO: just take data? */
static Map *currentProjectMap(int mapNum) {
  ProjectWindowData *data = projectWindow->data;
  return data->project.maps[mapNum];
}

/* TODO: move me somewhere, remove duplicate code from SongNamesEditor, EntityNamesEditor... */
static int listItemStart(int selected) {
  if(selected < 10) {
    return 2;
  } else if(selected < 100) {
    return 3;
  } else {
    return 4;
  }
}

/* TODO: just pass in data? */
static void updateCurrentProjectMapName(FrameworkWindow *projectWindow, int mapNum, Map *map) {
  ProjectWindowData *data = projectWindow->data;
  updateProjectMapName(&data->project, mapNum, map);
}

void updateCurrentProjectSongName(int songNum, char *name) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "updateCurrentProjectSongName: failed to get project window\n");
    goto error;
  }

  data = window->data;

  strcpy(&data->project.songNameStrs[songNum][listItemStart(songNum)], name);
  /* TODO: uhm, when is this called? is this correct? */
  data->projectSaved = TRUE;

done:
  return;

error:
  return;
}

void updateCurrentProjectEntityName(int entityNum, char *name) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "updateCurrentProjectEntityName: failed to get project window\n");
    goto error;
  }

  data = window->data;

  strcpy(&data->project.entityNameStrs[entityNum][listItemStart(entityNum)], name);
  /* TODO: uhm, when is this called? is this correct? */
  data->projectSaved = TRUE;

done:
  return;

error:
  return;
}

/* TODO: just pass in data? */
static char *currentProjectGetMapName(FrameworkWindow *projectWindow, int mapNum) {
  ProjectWindowData *data = projectWindow->data;

  Map *map = data->project.maps[mapNum];
  if(!map) {
    return NULL;
  }
  return map->name;
}

char *currentProjectGetSongName(int songNum) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "currentProjectGetSongName: failed to get project window\n");
    goto error;
  }

  data = window->data;

  return data->project.songNameStrs[songNum];

error:
  return NULL;
}

char *currentProjectGetEntityName(int entityNum) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "currentProjectGetEntityName: failed to get project window\n");
    goto error;
  }

  data = window->data;

  return data->project.entityNameStrs[entityNum];

error:
  return NULL;
}

struct List *currentProjectGetMapNames(void) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "currentProjectGetMapNames: failed to get project window\n");
    goto error;
  }

  data = window->data; 

  return &data->project.mapNames;

error:
  return NULL;
}

struct List *currentProjectGetSongNames(void) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "currentProjectGetSongNames: failed to get project window\n");
    goto error;
  }

  data = window->data;

  return &data->project.songNames;

error:
  return NULL;
}

struct List *currentProjectGetEntityNames(void) {
  ProjectWindowData *data;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "currentProjectGetEntityNames: failed to get project window\n");
    goto error;
  }

  data = window->data;

  return &data->project.entityNames;

error:
  return NULL;
}

int loadTilesetPackageFromFile(char *file) {
  TilesetPackage *newTilesetPackage;
  ProjectWindowData *data;
  FrameworkWindow *window = getProjectWindow();

  if(!window) {
    fprintf(stderr, "loadTIlesetPackageFromFile: failed to get project window\n");
    goto error;
  }

  data = window->data;

  newTilesetPackage = tilesetPackageLoadFromFile(file);
  if(!newTilesetPackage) {
    EasyRequest(
      window->intuitionWindow,
      &tilesetPackageLoadFailEasyStruct,
      NULL,
      file);
    goto error;
  }
  freeTilesetPackage(data->tilesetPackage);
  data->tilesetPackage = newTilesetPackage;
  setTilesetPackagePath(window, file);

  return 1;

error:
  return 0;
}

int loadTilesetPackageFromAsl(char *dir, char *file) {
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

static int confirmCreateMap(FrameworkWindow *projectWindow, int mapNum) {
  return EasyRequest(
    projectWindow->intuitionWindow,
    &confirmCreateMapEasyStruct,
    NULL,
    mapNum);
}

static BOOL currentProjectCreateMap(ProjectWindowData *data, int mapNum) {
  data->project.maps[mapNum] = allocMap();
  if(!data->project.maps[mapNum]) {
    fprintf(stderr, "currentProjectCreateMap: failed to allocate new map\n");
    goto error;
  }
  data->project.mapCnt++;
  data->projectSaved = 0;
  return TRUE;

error:
  return FALSE;
}

int openMapNum(FrameworkWindow *projectWindow, int mapNum) {
  MapEditor *mapEditor;

  if(!currentProjectHasMap(projectWindow, mapNum)) {
    if(!confirmCreateMap(projectWindow, mapNum)) {
      return 0;
    }

    if(!currentProjectCreateMap(projectWindow->data, mapNum)) {
      fprintf(stderr, "openMapNum: failed to create map\n");
      return 0;
    }
  }

  mapEditor = newMapEditorWithMap(currentProjectMap(mapNum), mapNum);
  if(!mapEditor) {
    fprintf(stderr, "openMapNum: failed to create new map editor\n");
    return 0;
  }

  addToMapEditorSet(mapEditor);
  /* TODO: fix me */
  /* addWindowToSet(mapEditor->window); */
  enableMapRevert(mapEditor);
  return 1;
}

void openMap(FrameworkWindow *projectWindow) {
  MapEditor *mapEditor;

  int selected = openMapRequester(projectWindow);
  if(!selected) {
    return;
  }
  selected--;

  mapEditor = findMapEditor(selected);
  if(mapEditor) {
    WindowToFront(mapEditor->window->intuitionWindow);
  } else {
    openMapNum(projectWindow, selected);
  }
}
