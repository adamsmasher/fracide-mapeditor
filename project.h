#ifndef FRAC_PROJECT_H
#define FRAC_PROJECT_H

#define MAP_TILES_WIDE 10
#define MAP_TILES_HIGH  9

#define TILESET_PACKAGE_PATH_SIZE 256

#include <exec/types.h>

typedef struct Map_tag {
	char name[64];
	UWORD tilesetNum;
	UBYTE tiles[MAP_TILES_WIDE * MAP_TILES_HIGH];
} Map;

typedef struct Project_tag {
	char tilesetPackagePath[TILESET_PACKAGE_PATH_SIZE];
	Map *maps[128];
} Project;

#endif
