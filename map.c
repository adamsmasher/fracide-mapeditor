#include "Map.h"

#include <stdlib.h>
#include <string.h>

Map *allocMap(void) {
	Map *map = malloc(sizeof(Map));
    if(!map) {
        return NULL;
    }

	map->name[0] = '\0';
	map->tilesetNum = 0;
	memset(map->tiles, 0, MAP_TILES_WIDE * MAP_TILES_HIGH);
	return map;
}

Map *copyMap(Map *oldMap) {
    Map *newMap = malloc(sizeof(Map));
    if(!newMap) {
        return NULL;
    }

    strcpy(newMap->name, oldMap->name);
    newMap->tilesetNum = oldMap->tilesetNum;
    memcpy(newMap->tiles, oldMap->tiles, MAP_TILES_WIDE * MAP_TILES_HIGH);
    return newMap;
}
