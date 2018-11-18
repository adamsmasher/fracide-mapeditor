#ifndef FRAC_PROJECT_H
#define FRAC_PROJECT_H

#include <exec/types.h>
#include <exec/lists.h>
#include "map.h"

#define TILESET_PACKAGE_PATH_SIZE 256
#define MAX_ENTITIES_IN_PROJECT 128
#define MAX_MAPS_IN_PROJECT 128

typedef struct Project_tag {
    char tilesetPackagePath[TILESET_PACKAGE_PATH_SIZE];
    UWORD mapCnt;
    Map *maps[128];
    char songNameStrs[128][80];
    char entityNameStrs[MAX_ENTITIES_IN_PROJECT][80];
} Project;

void initProject(Project*);
void freeProject(Project*);
void copyProject(Project *src, Project *dest);

int loadProjectFromFile(char *file, Project*);
int saveProjectToFile(Project*, char *file);

#endif
