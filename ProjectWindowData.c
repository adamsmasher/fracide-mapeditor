#include "ProjectWindowData.h"

#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

#include "Project.h"
#include "TilesetPackage.h"

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

#define PROJECT_FILENAME_LENGTH 256

struct ProjectWindowData_tag {
  Project project;
  BOOL projectSaved;
  char projectFilename[PROJECT_FILENAME_LENGTH];
  TilesetPackage *tilesetPackage;
  /* TODO: one day this should only exist inside the ListView gadget */
  struct List songNames;
};

static void initSongNameNodes(ProjectWindowData *data) {
  int i;
  struct Node *node, *next;

  NewList(&data->songNames);
  for(i = 0; i < 128; i++) {
    /* TODO: don't do this */
    sprintf(data->project.songNameStrs[i], "%d:", i);
    node = malloc(sizeof(struct Node));
    /* TODO: handle node creation failure */
    AddTail(&data->songNames, node);
  }

  node = data->songNames.lh_Head;
  i = 0;
  while(next = node->ln_Succ) {
    node->ln_Name = data->project.songNameStrs[i];
    node = next;
    i++;
  }
}

ProjectWindowData *createProjectData(void) {
  ProjectWindowData *data = malloc(sizeof(ProjectWindowData));
  if(!data) {
    fprintf(stderr, "createProjectData: failed to allocate project window data\n");
    goto error;
  }

  initProject(&data->project);
  data->projectSaved = TRUE;
  clearProjectDataFilename(data);
  data->tilesetPackage = NULL;

  initSongNameNodes(data);

  return data;
error:
  return NULL;
}

static void freeSongNames(ProjectWindowData *data) {
  struct Node *node, *next;

  node = data->songNames.lh_Head;
  while(next = node->ln_Succ) {
    free(node);
    node = next;
  }
}

static void clearProjectData(ProjectWindowData *data) {
  freeTilesetPackage(data->tilesetPackage);
  data->tilesetPackage = NULL;
  freeProject(&data->project);
  data->projectSaved = TRUE;

  freeSongNames(data);
}

void freeProjectData(ProjectWindowData *data) {
  clearProjectData(data);
  free(data);
}

void projectDataInitProject(ProjectWindowData *data) {
  clearProjectData(data);
  initProject(&data->project);
}

ProjectLoadResult projectDataLoadProjectFromFile(ProjectWindowData *data, char *filename) {
  Project *newProject;

  newProject = malloc(sizeof(Project));
  if(!newProject) {
    fprintf(stderr, "projectDataLoadProjectFromFile: failed to allocate project\n");
    goto error;
  }

  if(!loadProjectFromFile(filename, newProject)) {
    fprintf(stderr, "projectDataLoadProjectFromFile: failed to load project from file %s\n", filename);
    goto error_freeProject;
  }

  clearProjectData(data);
  copyProject(newProject, &data->project);
  free(newProject);

  if(*data->project.tilesetPackagePath && !projectDataLoadTilesetPackage(data, data->project.tilesetPackagePath)) {
    /* because the tileset will now be empty, we've changed from the
       saved version */
    data->projectSaved = FALSE;
    goto tileset_error;
  }

done:
  return PROJECT_LOAD_OK;

error_freeProject:
  free(newProject);
error:
  return PROJECT_LOAD_ERROR;

tileset_error:
  return PROJECT_LOAD_OK_TILESET_ERROR;
}

BOOL projectDataCreateMap(ProjectWindowData *data, int mapNum) {
  Map *map = allocMap();
  if(!map) {
    fprintf(stderr, "projectDataCreateMap: failed to allocate new map\n");
    goto error;
  }
  data->project.maps[mapNum] = map;
  data->project.mapCnt++;
  data->projectSaved = FALSE;

done:
  return TRUE;
error:
  return FALSE;
}

BOOL projectDataHasMap(ProjectWindowData *data, int mapNum) {
  return (BOOL)(data->project.maps[mapNum]);
}

Map *projectDataGetMap(ProjectWindowData *data, int mapNum) {
  return data->project.maps[mapNum];
}

struct List *projectDataGetMapNames(ProjectWindowData *data) {
  /* TODO: i suspect i want to move the LISTS into the data and out of the project */
  return &data->project.mapNames;
}

static BOOL projectDataSaveNewMap(ProjectWindowData *data, Map *map, int mapNum) {
  Map *mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "currentProjectSaveNewMap: couldn't allocate map copy\n");
    return FALSE;
  }
  data->project.mapCnt++;
  data->project.maps[mapNum] = mapCopy;

  projectUpdateMapName(&data->project, mapNum, map);

  return TRUE;
}

static void projectDataOverwriteMap(ProjectWindowData *data, Map *map, int mapNum) {
  overwriteMap(map, data->project.maps[mapNum]);
  projectUpdateMapName(&data->project, mapNum, map);
  data->projectSaved = FALSE;
}

BOOL projectDataSaveMap(ProjectWindowData *data, Map *map, int mapNum) {
  if(projectDataHasMap(data, mapNum)) {
    projectDataOverwriteMap(data, map, mapNum);
    return TRUE;
  } else {
    return projectDataSaveNewMap(data, map, mapNum);
  }
}

char *projectDataGetMapName(ProjectWindowData *data, int mapNum) {
  Map *map = data->project.maps[mapNum];
  if(!map) {
    return NULL;
  }
  return map->name;
}

struct List *projectDataGetEntityNames(ProjectWindowData *data) {
  /* TODO: i suspect i want to move the LISTS into the data and out of the project */
  return &data->project.entityNames;
}

char *projectDataGetEntityName(ProjectWindowData *data, int entityNum) {
  return data->project.entityNameStrs[entityNum];
}

void projectDataUpdateEntityName(ProjectWindowData *data, int entityNum, char *name) {
  strcpy(&data->project.entityNameStrs[entityNum][listItemStart(entityNum)], name);
  data->projectSaved = TRUE;
}

struct List *projectDataGetSongNames(ProjectWindowData *data) {
  /* TODO: i suspect i want to move the LISTS into the gadget */
  return &data->songNames;
}

char *projectDataGetSongName(ProjectWindowData *data, int songNum) {
  return data->project.songNameStrs[songNum];
}

void projectDataUpdateSongName(ProjectWindowData *data, int songNum, char *name) {
  strcpy(&data->project.songNameStrs[songNum][listItemStart(songNum)], name);
  data->projectSaved = TRUE;
}

BOOL projectDataIsSaved(ProjectWindowData *data) {
  return data->projectSaved;
}

BOOL projectDataSaveProjectToFile(ProjectWindowData *data, char *filename) {
  if(!saveProjectToFile(&data->project, filename)) {
    fprintf(stderr, "projectDataSaveProjectToFile: failed to save project to %s\n", filename);
    goto error;
  }
  data->projectSaved = TRUE;

done:
  return TRUE;

error:
  return FALSE;
}

void setProjectDataFilename(ProjectWindowData *data, char *filename) {
  strcpy(data->projectFilename, filename);
}

void clearProjectDataFilename(ProjectWindowData *data) {
  data->projectFilename[0] = '\0';
}

char *projectDataGetFilename(ProjectWindowData *data) {
  if(data->projectFilename[0]) {
    return data->projectFilename;
  } else {
    return NULL;
  }
}

BOOL projectDataHasTilesetPackage(ProjectWindowData *data) {
  return (BOOL)(data->tilesetPackage);
}

BOOL projectDataLoadTilesetPackage(ProjectWindowData *data, char *filename) {
  TilesetPackage *newTilesetPackage = tilesetPackageLoadFromFile(filename);
  if(!newTilesetPackage) {
    goto error;
  }

  freeTilesetPackage(data->tilesetPackage);
  data->tilesetPackage = newTilesetPackage;
  strcpy(data->project.tilesetPackagePath, filename);
  data->projectSaved = FALSE;

done:
  return TRUE;

error:
  return FALSE;
}

char *projectDataGetTilesetPath(ProjectWindowData *data) {
  return data->project.tilesetPackagePath;
}

char *projectDataGetTilesetName(ProjectWindowData *data, UWORD tilesetNumber) {
  return data->tilesetPackage->tilesetPackageFile.tilesetNames[tilesetNumber];
}

WORD projectDataGetTilesetCount(ProjectWindowData *data) {
  return data->tilesetPackage->tilesetPackageFile.tilesetCnt;
}

TilesetImgs *projectDataGetTilesetImgs(ProjectWindowData *data, UWORD tilesetNumber) {
  return &data->tilesetPackage->tilesetPackageFile.tilesetImgs[tilesetNumber];
}
