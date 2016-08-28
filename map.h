#ifndef MAP_H
#define MAP_H

#include <exec/types.h>

#define MAP_TILES_WIDE 10
#define MAP_TILES_HIGH  9

typedef struct Map_tag {
	/* 1-indexed */
	UWORD mapNum;
	char name[64];
	UWORD tilesetNum;
	UBYTE tiles[MAP_TILES_WIDE * MAP_TILES_HIGH];
} Map;

Map *allocMap(void);

#endif
