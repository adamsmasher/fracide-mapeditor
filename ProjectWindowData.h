#ifndef PROJECT_WINDOW_DATA_H
#define PROJECT_WINDOW_DATA_H

#include <exec/types.h>

#include "Map.h"
#include "TilesetPackage.h"

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

BOOL projectDataCreateMap(ProjectWindowData*, UWORD mapNum);
BOOL projectDataHasMap(ProjectWindowData*, UWORD mapNum);
Map *projectDataGetMap(ProjectWindowData*, UWORD mapNum);
char *projectDataGetMapName(ProjectWindowData*, UWORD mapNum);

/* returns TRUE on success, FALSE on failure */
BOOL projectDataSaveMap(ProjectWindowData*, Map*, UWORD mapNum);

char *projectDataGetEntityName(ProjectWindowData*, UWORD entityNum);
void projectDataUpdateEntityName(ProjectWindowData*, UWORD entityNum, char*);

char *projectDataGetSongName(ProjectWindowData*, UWORD songNum);
void projectDataUpdateSongName(ProjectWindowData*, UWORD songNum, char*);

BOOL projectDataIsSaved(ProjectWindowData*);

BOOL projectDataSaveProjectToFile(ProjectWindowData*, char*);

void setProjectDataFilename(ProjectWindowData*, char*);
void clearProjectDataFilename(ProjectWindowData*);
char *projectDataGetFilename(ProjectWindowData *data);

BOOL projectDataHasTilesetPackage(ProjectWindowData*);
BOOL projectDataLoadTilesetPackage(ProjectWindowData*, char*);
char *projectDataGetTilesetPath(ProjectWindowData*);
char *projectDataGetTilesetName(ProjectWindowData*, UWORD tilesetNumber);
WORD projectDataGetTilesetCount(ProjectWindowData*);
TilesetImgs *projectDataGetTilesetImgs(ProjectWindowData*, UWORD tilesetNumber);

#endif
