#ifndef FRAC_CURRENT_PROJECT_H
#define FRAC_CURRENT_PROJECT_H

#include <exec/types.h>
#include "Project.h"

/* TODO: rename all these to CURRENT project */

BOOL ensureProjectSaved(void);

void clearProject(void);

void setProjectFilename(char*);
char *getProjectFilename(void);

void setTilesetPackagePath(char*);

void openProjectFromAsl(char *dir, char *file);
void openProjectFromFile(char*);

int saveProject(void);
int saveProjectAs(void);

BOOL currentProjectCreateMap(int mapNum);

/* TODO: these signatures are weird */
void updateCurrentProjectMapName(int mapNum, Map*);
void updateCurrentProjectSongName(int songNum, char*);
void updateCurrentProjectEntityName(int entityNum, char*);

/* TODO: don't expose us */
extern Project project;

#endif
