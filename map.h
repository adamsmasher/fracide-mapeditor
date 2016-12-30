#ifndef MAP_H
#define MAP_H

#include <stdio.h>
#include <exec/types.h>

#include "Entity.h"

#define MAP_TILES_WIDE 10
#define MAP_TILES_HIGH  9

#define MAX_ENTITIES_PER_MAP 8

typedef struct Map_tag {
    char name[64];

    /* 1-indexed */
    UWORD tilesetNum;

    /* 1-indexed */
    UWORD songNum;

    UBYTE tiles[MAP_TILES_WIDE * MAP_TILES_HIGH];

    Entity entities[MAX_ENTITIES_PER_MAP];
    UWORD entityCnt;
} Map;

Map *allocMap(void);
Map *copyMap(Map*);
void overwriteMap(Map *srcMap, Map *destMap);

void writeMap(Map*, FILE*);
int readMap(Map*, FILE*);

void mapAddNewEntity(Map*);
void mapRemoveEntity(Map*, int entityNum);

#endif
