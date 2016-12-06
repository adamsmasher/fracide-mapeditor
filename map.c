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
    map->songNum = 0;
    map->entityCnt = 0;
    memset(map->tiles, 0, MAP_TILES_WIDE * MAP_TILES_HIGH);
    return map;
}

void overwriteMap(Map *srcMap, Map *destMap) {
    strcpy(destMap->name, srcMap->name);
    destMap->tilesetNum = srcMap->tilesetNum;
    destMap->songNum = srcMap->songNum;
    destMap->entityCnt = srcMap->entityCnt;
    memcpy(destMap->tiles, srcMap->tiles, MAP_TILES_WIDE * MAP_TILES_HIGH);
    memcpy(destMap->entities, srcMap->entities, sizeof(Entity) * srcMap->entityCnt);
}

Map *copyMap(Map *oldMap) {
    Map *newMap = malloc(sizeof(Map));
    if(!newMap) {
        return NULL;
    }
    overwriteMap(oldMap, newMap);

    return newMap;
}

void mapAddNewEntity(Map *map) {
    Entity *entity = &map->entities[map->entityCnt];
    map->entityCnt++;

    entity->entityNum = 0;
    entity->row = 0;
    entity->col = 0;
    entity->vramSlot = 0;
}

void writeMap(Map *map, FILE *fp) {
    fwrite(map->name, 1, 64, fp);
    fwrite(&map->tilesetNum, 2, 1, fp);
    fwrite(&map->songNum, 2, 1, fp);
    fwrite(map->tiles, 1, 90, fp);
    fwrite(map->entities, sizeof(Entity), MAX_ENTITIES_PER_MAP, fp);
    fwrite(&map->entityCnt, 2, 1, fp);
}

int readMap(Map *map, FILE *fp) {
    if(fread(map->name, 1, 64, fp) != 64) {
        fprintf(stderr, "readMap: couldn't read map name\n");
        return 0;
    }

    if(fread(&map->tilesetNum, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read tileset num\n");
        return 0;
    }

    if(fread(&map->songNum, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read song num\n");
        return 0;
    }

    if(fread(map->tiles, 1, 90, fp) != 90) {
        fprintf(stderr, "readMap: couldn't read tiles\n");
        return 0;
    }

    if(fread(map->entities, sizeof(Entity), MAX_ENTITIES_PER_MAP, fp) != MAX_ENTITIES_PER_MAP) {
        fprintf(stderr, "readMap: couldn't read entities\n");
        return 0;
    }

    if(fread(&map->entityCnt, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read entity count\n");
        return 0;
    }

    /* TODO: allocate labels */

    return 1;
}
