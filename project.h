#ifndef FRAC_PROJECT_H
#define FRAC_PROJECT_H

#include <exec/types.h>
#include "map.h"

#define TILESET_PACKAGE_PATH_SIZE 256

typedef struct Project_tag {
	char tilesetPackagePath[TILESET_PACKAGE_PATH_SIZE];
	UWORD mapCnt;
	Map *maps[128];
} Project;

void initProject(Project*);
void freeProject(Project*);

int loadProjectFromFile(char *file, Project*);
int saveProjectToFile(char *file);

#endif