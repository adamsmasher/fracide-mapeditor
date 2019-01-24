#include "ProjectWindowData.h"

#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

#include "Export.h"
#include "Project.h"
#include "TilesetPackage.h"

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

BOOL projectDataExport(ProjectWindowData *data, FILE *fp) {
  return exportProject(&data->project, fp);
}

BOOL projectDataCreateMap(ProjectWindowData *data, UWORD mapNum) {
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

BOOL projectDataHasMap(ProjectWindowData *data, UWORD mapNum) {
  return (BOOL)(data->project.maps[mapNum]);
}

Map *projectDataGetMap(ProjectWindowData *data, UWORD mapNum) {
  return data->project.maps[mapNum];
}

static BOOL projectDataSaveNewMap(ProjectWindowData *data, Map *map, UWORD mapNum) {
  Map *mapCopy = copyMap(map);
  if(!mapCopy) {
    fprintf(stderr, "currentProjectSaveNewMap: couldn't allocate map copy\n");
    return FALSE;
  }
  data->project.mapCnt++;
  data->project.maps[mapNum] = mapCopy;

  return TRUE;
}

static void projectDataOverwriteMap(ProjectWindowData *data, Map *map, UWORD mapNum) {
  overwriteMap(map, data->project.maps[mapNum]);
  data->projectSaved = FALSE;
}

BOOL projectDataSaveMap(ProjectWindowData *data, Map *map, UWORD mapNum) {
  if(projectDataHasMap(data, mapNum)) {
    projectDataOverwriteMap(data, map, mapNum);
    return TRUE;
  } else {
    return projectDataSaveNewMap(data, map, mapNum);
  }
}

char *projectDataGetMapName(ProjectWindowData *data, UWORD mapNum) {
  Map *map = data->project.maps[mapNum];
  if(!map) {
    return NULL;
  }
  return map->name;
}

char *projectDataGetEntityName(ProjectWindowData *data, UWORD entityNum) {
  return data->project.entityNameStrs[entityNum];
}

void projectDataUpdateEntityName(ProjectWindowData *data, UWORD entityNum, char *name) {
  strcpy(data->project.entityNameStrs[entityNum], name);
  data->projectSaved = FALSE;
}

char *projectDataGetSongName(ProjectWindowData *data, UWORD songNum) {
  return data->project.songNameStrs[songNum];
}

void projectDataUpdateSongName(ProjectWindowData *data, UWORD songNum, char *name) {
  strcpy(data->project.songNameStrs[songNum], name);
  data->projectSaved = FALSE;
}

BOOL projectDataIsSaved(ProjectWindowData *data) {
  return data->projectSaved;
}

void projectDataSaveProjectToFile(ProjectWindowData *data, FILE *fp) {
  saveProjectToFile(&data->project, fp);
  data->projectSaved = TRUE;
}

void setProjectDataFilename(ProjectWindowData *data, const char *filename) {
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
