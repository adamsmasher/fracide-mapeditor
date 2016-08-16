#ifndef FRAC_PROJECT_H
#define FRAC_PROJECT_H

#define MAP_TILES_WIDE 10
#define MAP_TILES_HIGH  9

typedef struct Map_tag {
	char name[64];
	UWORD tilesetNum;
	UBYTE tiles[MAP_TILES_WIDE * MAP_TILES_HIGH];
} Map;

typedef struct Project_tag {
	char *tilesetPackageName;
	Map *maps[128];
} Project;

#endif
