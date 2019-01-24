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

#include "asl_util.h"
#include "easystructs.h"
#include "EntityRequester.h"
#include "MapEditor.h"
#include "MapEditorData.h"
#include "MapRequester.h"
#include "Project.h"
#include "ProjectWindowData.h"
#include "ProjectWindowMenu.h"

/* displays a message warning that the current project isn't saved in
   response to a destructive action (like quiting, or opening a new project) */
/* returns TRUE if the operation should proceed (if the user clicks 'Save'
   and the project is safely saved, or if the user clicks 'Don't Save' because
   they don't care about losing changes) */
/* returns FALSE if the operation should be aborted (if the user clicks 'Cancel',
   or if they try to 'Save' but for some reason that fails) */
static BOOL unsavedProjectAlert(FrameworkWindow *projectWindow) {
  int response;

  response = EasyRequest(
    projectWindow->intuitionWindow,
    &unsavedProjectAlertEasyStruct,
    NULL);

  switch(response) {
    case 0:
      /* cancel */
      return FALSE;
    case 1:
      /* save - if the save was a success returns TRUE */
      return projectWindowSaveProject(projectWindow);
    case 2:
      /* don't save - proceed anyway, so we return TRUE */
      return TRUE;
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
    if(isMapEditor(i)) {
      if(!mapEditorEnsureSaved(i->data)) {
        return FALSE;
      }
    }
    i = i->next;
  }
  return TRUE;
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
  (MenuSpec*)        NULL, /* set before creation */
  (GadgetBuilder)    NULL,
  (GadgetFreer)      NULL,
  (RefreshFunction)  NULL,
  (CanCloseFunction) ensureEverythingSaved,
  (CloseFunction)    onClose,
  (ClickFunction)    NULL
};

static void makeWindowFullScreen(void) {
  struct NewWindow *newWindow = &projectWindowKind.newWindow;
  newWindow->MinWidth  = newWindow->Width  = getScreenWidth();
  newWindow->MinHeight = newWindow->Height = getScreenHeight();
}

FrameworkWindow *newProjectWindow(void) {
  FrameworkWindow *projectWindow;
  ProjectWindowData *data;

  makeWindowFullScreen();

  data = createProjectData();
  if(!data) {
    fprintf(stderr, "newProjectWindow: failed to allocate data\n");
    goto error;
  }

  projectWindowKind.menuSpec = projectWindowMenuSpec;

  projectWindow = openWindowOnGlobalScreen(&projectWindowKind, data);
  if(!projectWindow) {
    fprintf(stderr, "newProjectWindow: failed to open window!\n");
    goto error_freeData;
  }

  ActivateWindow(projectWindow->intuitionWindow);

  return projectWindow;

error_freeData:
  freeProjectData(data);
error:
  return NULL;
}

static void setProjectFilename(FrameworkWindow *projectWindow, const char *filename) {
  setProjectDataFilename(projectWindow->data, filename);
  projectWindowMenuEnableRevertProject(projectWindow);
}

static void clearProjectFilename(FrameworkWindow *projectWindow) {
  clearProjectDataFilename(projectWindow->data);
  projectWindowMenuDisableRevertProject(projectWindow);
}

static BOOL saveProjectToFilename(FrameworkWindow *projectWindow, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if(!fp) {
    fprintf(stderr, "saveProjectToAsl: couldn't open file\n");
    goto error;
  }

  projectDataSaveProjectToFile(projectWindow->data, fp);
  setProjectFilename(projectWindow, filename);

  fclose(fp);
  return TRUE;

error:
  return FALSE;
}

static BOOL saveProjectToAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
  char *filename = aslFilename(dir, file);
  if(!filename) {
    fprintf(stderr, "saveProjectToAsl: couldn't make filename\n");
    goto error;
  }

  if(!saveProjectToFilename(projectWindow, filename)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &projectSaveFailEasyStruct,
      NULL,
      filename
    );
    goto error_freeFilename;
  }

  free(filename);
  return TRUE;

error_freeFilename:
  free(filename);
error:
  return FALSE;
}

BOOL projectWindowSaveProjectAs(FrameworkWindow *projectWindow) {
  BOOL result;
  struct FileRequester *request; 

  request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Save Project As",
    ASL_Window, projectWindow->intuitionWindow,
    ASL_FuncFlags, FILF_SAVE,
    TAG_END);
  if(!request) {
    fprintf(stderr, "projectWindowSaveProjectAs: couldn't allocate asl request tags\n");
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

BOOL projectWindowSaveProject(FrameworkWindow *projectWindow) {
  const char *filename = projectDataGetFilename(projectWindow->data);
  if(filename) {
    if(!saveProjectToFilename(projectWindow, filename)) {
      EasyRequest(
        projectWindow->intuitionWindow,
        &projectSaveFailEasyStruct,
        NULL,
        filename);
      return FALSE;
    }
    return TRUE;
  } else {
    return projectWindowSaveProjectAs(projectWindow);
  }
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

void projectWindowNewProject(FrameworkWindow *projectWindow) {
  if(ensureEverythingSaved(projectWindow)) {
    projectDataInitProject(projectWindow->data);
    clearProjectFilename(projectWindow);
  }
}

void projectWindowOpenProject(FrameworkWindow *projectWindow) {
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

void projectWindowRevertProject(FrameworkWindow *projectWindow) {
  if(!confirmRevertProject(projectWindow)) {
    goto done;
  }

  openProjectFromFile(projectWindow, projectDataGetFilename(projectWindow->data));

done:
  return;
}

static void refreshAllTileDisplays(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditor(i)) {
      mapEditorRefreshTileDisplays(i);
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

void projectWindowSelectTilesetPackage(FrameworkWindow *projectWindow) {
  struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Select Tileset Package",
    ASL_Window, projectWindow->intuitionWindow,
    TAG_END);
  if(!request) {
    fprintf(stderr, "projectWindowSelectTilesetPackage: failed to allocate requester\n");
    goto error;
  }

  if(AslRequest(request, NULL)) {
    if(loadTilesetPackageFromAsl(projectWindow, request->rf_Dir, request->rf_File)) {
      refreshAllTileDisplays(projectWindow);
    }
  }

  FreeAslRequest(request);

done:
  return;
error:
  return;
}

void projectWindowQuit(FrameworkWindow *projectWindow) {
  tryToCloseWindow(projectWindow);
}

static int confirmCreateMap(FrameworkWindow *projectWindow, int mapNum) {
  return EasyRequest(
    projectWindow->intuitionWindow,
    &confirmCreateMapEasyStruct,
    NULL,
    mapNum);
}

BOOL projectWindowOpenMapNum(FrameworkWindow *projectWindow, int mapNum) {
  FrameworkWindow *mapEditor;

  if(!projectDataHasMap(projectWindow->data, mapNum)) {
    if(!confirmCreateMap(projectWindow, mapNum)) {
      goto error;
    }

    if(!projectDataCreateMap(projectWindow->data, mapNum)) {
      fprintf(stderr, "projectWindowOpenMapNum: failed to create map\n");
      goto error;
    }
  }

  mapEditor = newMapEditorWithMap(projectWindow, projectDataGetMap(projectWindow->data, mapNum), mapNum);
  if(!mapEditor) {
    fprintf(stderr, "projectWindowOpenMapNum: failed to create new map editor\n");
    goto error;
  }

done:
  return TRUE;

error:
  return FALSE;
}

static FrameworkWindow *findMapEditor(FrameworkWindow *projectWindow, int mapNum) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditor(i)) {
      MapEditorData *data = i->data;
      if(mapEditorDataGetMapNum(data) == mapNum) {
        return i;
      }
    }
    i = i->next;
  }
  return NULL;
}

void projectWindowNewMap(FrameworkWindow *projectWindow) {
  FrameworkWindow *mapEditor = newMapEditorNewMap(projectWindow);
  if(!mapEditor) {
    fprintf(stderr, "projectWindowNewMap: failed to create mapEditor\n");
    return;
  }
}

static UWORD openMapRequester(FrameworkWindow *projectWindow) {
  return spawnMapRequester(projectWindow, "Open Map");
}

void projectWindowOpenMap(FrameworkWindow *projectWindow) {
  FrameworkWindow *mapEditor;

  UWORD selected = openMapRequester(projectWindow);
  if(!selected) {
    return;
  }
  selected--;

  mapEditor = findMapEditor(projectWindow, selected);
  if(mapEditor) {
    WindowToFront(mapEditor->intuitionWindow);
  } else {
    projectWindowOpenMapNum(projectWindow, selected);
  }
}

void projectWindowRefreshAllSongDisplays(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditor(i)) {
      mapEditorRefreshSongDisplays(i);
    }
    i = i->next;
  }
}

void projectWindowRefreshAllEntityBrowsers(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isMapEditor(i)) {
      mapEditorRefreshEntityBrowser(i);
    }
    i = i->next;
  }
}

static FrameworkWindow *findEntityNamesEditor(FrameworkWindow *parent) {
  FrameworkWindow *i = parent->children;
  while(i) {
    if(isEntityNamesEditor(i)) {
      return i;
    }
    i = i->next;
  }
  return NULL;
}

void projectWindowShowEntityNamesEditor(FrameworkWindow *projectWindow) {
  FrameworkWindow *entityNamesEditor = findEntityNamesEditor(projectWindow);
  if(entityNamesEditor) {
    WindowToFront(entityNamesEditor->intuitionWindow);
  } else {
    entityNamesEditor = newEntityNamesEditor(projectWindow);
  }
}

static FrameworkWindow *projectWindowFindSongNamesEditor(FrameworkWindow *projectWindow) {
  FrameworkWindow *i = projectWindow->children;
  while(i) {
    if(isSongNamesEditor(i)) {
      return i;
    }
  }
  return NULL;
}

void projectWindowShowSongNamesEditor(FrameworkWindow *projectWindow) {
  FrameworkWindow *songNamesEditor = projectWindowFindSongNamesEditor(projectWindow);
  if(songNamesEditor) {
    WindowToFront(songNamesEditor->intuitionWindow);
  } else {
    newSongNamesEditor(projectWindow);
  }
}

static BOOL exportProjectToFile(FrameworkWindow *projectWindow, char *file) {
  FILE *fp = fopen(file, "wb");
  if(!fp) {
    fprintf(stderr, "exportProjectToFile: failed to open file %s\n", file);
    goto error;
  }

  if(!projectDataExport(projectWindow->data, fp)) {
    fprintf(stderr, "exportProjectToFile: failed to export data\n");
    goto error_closeFile;
  }

  fclose(fp);
  return TRUE;

error_closeFile:
  fclose(fp);
error:
  return FALSE;
}

/* TODO: so much dupe code :( */
static void exportProjectToAsl(FrameworkWindow *projectWindow, char *dir, char *file) {
  size_t bufferLen = strlen(dir) + strlen(file) + 2;
  char *buffer = malloc(bufferLen);

  if(!buffer) {
    fprintf(
      stderr,
      "exportProjectToAsl: failed to allocate buffer "
      "(dir: %s) (file: %s)\n",
      dir  ? dir  : "NULL",
      file ? file : "NULL");
    goto error;
  }

  strcpy(buffer, dir);
  if(!AddPart(buffer, file, (ULONG)bufferLen)) {
    fprintf(
      stderr,
      "exportProjectToAsl: failed to add part "
      "(buffer: %s) (file: %s) (len: %d)\n",
      buffer ? buffer : "NULL",
      file   ? file   : "NULL",
      bufferLen);
    goto error_freeBuffer;
  }

  if(!exportProjectToFile(projectWindow, buffer)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &projectExportFailEasyStruct,
      NULL,
      buffer);
    goto error_freeBuffer;
  }

freeBuffer:
  free(buffer);
done:
  return;

error_freeBuffer:
  free(buffer);
error:
  return;
}

void projectWindowExport(FrameworkWindow *projectWindow) {
  struct FileRequester *request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Export project...",
    ASL_Window, projectWindow->intuitionWindow,
    ASL_FuncFlags, FILF_SAVE,
    TAG_END);
  if(!request) {
    fprintf(stderr, "projectWindowExport: failed to allocate requester\n");
    goto error;
  }

  if(AslRequest(request, NULL)) {
    exportProjectToAsl(projectWindow, request->rf_Dir, request->rf_File);
  }

  FreeAslRequest(request);

done:
  return;
error:
  return;
}
