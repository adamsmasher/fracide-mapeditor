#include "ProjectWindowData.h"

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
};

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

  return data;
error:
  return NULL;
}

static void clearProjectData(ProjectWindowData *data) {
  freeTilesetPackage(data->tilesetPackage);
  data->tilesetPackage = NULL;
  freeProject(&data->project);
  data->projectSaved = TRUE;
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
  /* TODO: i suspect i want to move the LISTS into the data and out of the project */
  return &data->project.songNames;
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
