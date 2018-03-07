#ifndef PROJECT_WINDOW_DATA_H
#define PROJECT_WINDOW_DATA_H

#include <exec/types.h>

#include "Map.h"

typedef struct ProjectWindowData_tag ProjectWindowData;

ProjectWindowData *createProjectData(void);
void freeProjectData(ProjectWindowData*);

typedef enum ProjectLoadResult_tag {
  PROJECT_LOAD_ERROR,
  PROJECT_LOAD_OK,
  PROJECT_LOAD_OK_TILESET_ERROR
} ProjectLoadResult;

void projectDataInitProject(ProjectWindowData*);
ProjectLoadResult projectDataLoadProjectFromFile(ProjectWindowData*, char*);

BOOL projectDataCreateMap(ProjectWindowData*, int mapNum);
BOOL projectDataHasMap(ProjectWindowData*, int mapNum);
Map *projectDataGetMap(ProjectWindowData*, int mapNum);
struct List *projectDataGetMapNames(ProjectWindowData*);
char *projectDataGetMapName(ProjectWindowData*, int mapNum);

struct List *projectDataGetEntityNames(ProjectWindowData*);
char *projectDataGetEntityName(ProjectWindowData*, int entityNum);
void projectDataUpdateEntityName(ProjectWindowData*, int entityNum, char*);

struct List *projectDataGetSongNames(ProjectWindowData*);
char *projectDataGetSongName(ProjectWindowData*, int songNum);
void projectDataUpdateSongName(ProjectWindowData*, int songNum, char*);

BOOL projectDataIsSaved(ProjectWindowData*);

BOOL projectDataSaveProjectToFile(ProjectWindowData*, char*);

void setProjectDataFilename(ProjectWindowData*, char*);
void clearProjectDataFilename(ProjectWindowData*);
char *projectDataGetFilename(ProjectWindowData *data);

BOOL projectDataLoadTilesetPackage(ProjectWindowData*, char*);
char *projectDataGetTilesetPath(ProjectWindowData*);

#endif