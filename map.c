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

void overwriteMap(Map *srcMap, Map *destMap) {
    strcpy(destMap->name, srcMap->name);
    destMap->tilesetNum = srcMap->tilesetNum;
    memcpy(destMap->tiles, srcMap->tiles, MAP_TILES_WIDE * MAP_TILES_HIGH);
}

Map *copyMap(Map *oldMap) {
    Map *newMap = malloc(sizeof(Map));
    if(!newMap) {
        return NULL;
    }
    overwriteMap(oldMap, newMap);

    return newMap;
}
