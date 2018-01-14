#ifndef FRAC_CURRENT_PROJECT_H
#define FRAC_CURRENT_PROJECT_H

#include <exec/types.h>
#include "Project.h"

void initCurrentProject(void);
void freeCurrentProject(void);

/* TODO: rename all these to CURRENT project */
/* TODO: add some order here */

BOOL ensureProjectSaved(void);

void clearProject(void);

void setProjectFilename(char*);
char *getProjectFilename(void);

void currentProjectSetTilesetPackagePath(char*);

void openProjectFromAsl(char *dir, char *file);
void openProjectFromFile(char*);

int saveProject(void);
int saveProjectAs(void);

BOOL currentProjectCreateMap(int mapNum);
BOOL currentProjectHasMap(int mapNum);
Map *currentProjectMap(int mapNum);
BOOL currentProjectSaveNewMap(Map*, int mapNum);
void currentProjectOverwriteMap(Map*, int mapNum);

/* TODO: these signatures are weird */
void updateCurrentProjectMapName(int mapNum, Map*);
void updateCurrentProjectSongName(int songNum, char*);
void updateCurrentProjectEntityName(int entityNum, char*);

/* TODO: this is a bit of a code smell */
struct List *currentProjectGetMapNames(void);
struct List *currentProjectGetSongNames(void);
struct List *currentProjectGetEntityNames(void);

char *currentProjectGetMapName(int mapNum);
char *currentProjectGetSongName(int songNum);
char *currentProjectGetENtityName(int entityNum);

#endif
