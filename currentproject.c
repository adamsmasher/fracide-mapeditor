#include "currentproject.h"

#include <libraries/asl.h>
#include <proto/asl.h>

#include <libraries/dos.h>
#include <proto/dos.h>

#include <proto/intuition.h>

#include <stdlib.h>
#include <string.h>

#include "currenttiles.h"
#include "easystructs.h"
#include "globals.h"
#include "mapeditorset.h"
#include "menu.h"
#include "ProjectWindow.h"

#define PROJECT_FILENAME_LENGTH 256

static Project project;
static BOOL projectSaved = 1;
static char projectFilename[PROJECT_FILENAME_LENGTH];

/* TODO: many of these should be projectwindow methods */

void initCurrentProject(void) {
  initProject(&project);
}

void freeCurrentProject(void) {
  freeProject(&project);
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

  if(!saveProjectToFile(&project, buffer)) {
    EasyRequest(
      projectWindow->intuitionWindow,
      &projectSaveFailEasyStruct,
      NULL,
      buffer);
    goto error_freeBuffer;
  }
  setProjectFilename(buffer);

  projectSaved = TRUE;

freeBuffer:
  free(buffer);
done:
  return TRUE;

error_freeBuffer:
  free(buffer);
error:
  return FALSE;
}

BOOL saveProjectAs(void) {
  BOOL result;
  struct FileRequester *request; 

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "saveProjectAs: couldn't get project window\n");
    goto error;
  }

  request = AllocAslRequestTags(ASL_FileRequest,
    ASL_Hail, "Save Project As",
    ASL_Window, window->intuitionWindow,
    ASL_FuncFlags, FILF_SAVE,
    TAG_END);
  if(!request) {
    fprintf(stderr, "saveProjectAs: couldn't allocate asl request tags\n");
    goto error;
  }

  result = AslRequest(request, NULL);
  if(result) {
    result = saveProjectToAsl(window, request->rf_Dir, request->rf_File);
  }

  FreeAslRequest(request);
done:
  return result;
error:
  return FALSE;
}

BOOL saveProject(void) {
  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "saveProject: couldn't get project window\n");
    goto error;
  }

  if(*projectFilename) {
    if(!saveProjectToFile(&project, projectFilename)) {
      EasyRequest(
        window->intuitionWindow,
        &projectSaveFailEasyStruct,
        NULL,
        projectFilename);
      goto error;
    } else {
      return TRUE;
    }
  } else {
    return saveProjectAs();
  }

error:
  return FALSE;
}

static BOOL unsavedProjectAlert(void) {
  int response;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "unsavedProjectAlert: couldn't get project window\n");
    goto error;
  }

  response = EasyRequest(
    window->intuitionWindow,
    &unsavedProjectAlertEasyStruct,
    NULL);

  switch(response) {
    case 0: return FALSE;
    case 1: return saveProject();
    case 2: return TRUE;
    default:
      fprintf(stderr, "unsavedProjectAlert: unknown response %d\n", response);
      goto error;
  }
  
error:
  return FALSE;
}

BOOL ensureProjectSaved(void) {
  return (BOOL)(projectSaved || unsavedProjectAlert());
}

void clearProject(void) {
    closeAllMapEditors();
    freeTilesetPackage(tilesetPackage);
    tilesetPackage = NULL;
    freeProject(&project);
    projectSaved = 1;
}

void setProjectFilename(char *filename) {
  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "setProjectFilename: couldn't get project window");
    goto error;
  }

  /* TODO: honestly these OnMenu/OffMenu things could probably be made into functions elsewhere */
  if(filename) {
    strcpy(projectFilename, filename);
    OnMenu(window->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
  } else {
    projectFilename[0] = '\0';
    OffMenu(window->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
  }
done:
  return;

error:
  return;
}

char *getProjectFilename(void) {
  return projectFilename;
}

void openProjectFromFile(char *file) {
  Project *myNewProject;

  FrameworkWindow *window = getProjectWindow();
  if(!window) {
    fprintf(stderr, "openProjectFromFile: couldn't get project window\n");
    goto error;
  }

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

  clearProject();
  copyProject(myNewProject, &project);
  setProjectFilename(file);

  if(*project.tilesetPackagePath && !loadTilesetPackageFromFile(project.tilesetPackagePath)) {
    EasyRequest(
      window->intuitionWindow,
      &tilesetPackageLoadFailEasyStruct,
      NULL,
      project.tilesetPackagePath);

      /* because the tileset will now be empty, we've changed from the
         saved version */
      projectSaved = FALSE;
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

void openProjectFromAsl(char *dir, char *file) {
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

void currentProjectSetTilesetPackagePath(char *file) {
    strcpy(project.tilesetPackagePath, file);
    projectSaved = 0;
}

BOOL currentProjectCreateMap(int mapNum) {
  project.maps[mapNum] = allocMap();
  if(!project.maps[mapNum]) {
    fprintf(stderr, "openMapNum: failed to allocate new map\n");
    return FALSE;
  }
  project.mapCnt++;
  projectSaved = 0;
  return TRUE;
}

/* TODO: this is weird */
BOOL currentProjectSaveNewMap(Map *map, int mapNum) {
  Map *mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "currentProjectSaveMap: couldn't allocate map copy\n");
    return FALSE;
  }
  project.mapCnt++;
  project.maps[mapNum] = mapCopy;
  return TRUE;
}

void currentProjectOverwriteMap(Map *map, int mapNum) {
  overwriteMap(map, project.maps[mapNum]);
}

BOOL currentProjectHasMap(int mapNum) {
  return (BOOL)(project.maps[mapNum] ? TRUE : FALSE);
}

Map *currentProjectMap(int mapNum) {
  return project.maps[mapNum];
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

void updateCurrentProjectMapName(int mapNum, Map *map) {
  updateProjectMapName(&project, mapNum, map);
}

void updateCurrentProjectSongName(int songNum, char *name) {
  strcpy(&project.songNameStrs[songNum][listItemStart(songNum)], name);
  projectSaved = 1;
}

void updateCurrentProjectEntityName(int entityNum, char *name) {
  strcpy(&project.entityNameStrs[entityNum][listItemStart(entityNum)], name);
  projectSaved = 1;
}

char *currentProjectGetMapName(int mapNum) {
  Map *map = project.maps[mapNum];
  if(!map) {
    return NULL;
  }
  return map->name;
}

char *currentProjectGetSongName(int songNum) {
  return project.songNameStrs[songNum];
}

char *currentProjectGetEntityName(int entityNum) {
  return project.entityNameStrs[entityNum];
}

struct List *currentProjectGetMapNames(void) {
  return &project.mapNames;
}

struct List *currentProjectGetSongNames(void) {
  return &project.songNames;
}

struct List *currentProjectGetEntityNames(void) {
  return &project.entityNames;
}
