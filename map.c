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

Entity *mapAddNewEntity(Map *map) {
  Entity *entity = &map->entities[map->entityCnt];

  map->entityCnt++;

  entity->entityNum = 0;
  entity->row = 0;
  entity->col = 0;
  entity->vramSlot = 0;
  entity->tagCnt = 0;

  return entity;
}

void mapRemoveEntity(Map *map, int entityNum) {
    map->entityCnt--;

    /* copy the entities backwards */
    memmove(
        &map->entities[entityNum],
        &map->entities[entityNum + 1],
        sizeof(Entity) * (map->entityCnt - entityNum));
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
        goto error;
    }

    if(fread(&map->tilesetNum, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read tileset num\n");
        goto error;
    }

    if(fread(&map->songNum, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read song num\n");
        goto error;
    }

    if(fread(map->tiles, 1, 90, fp) != 90) {
        fprintf(stderr, "readMap: couldn't read tiles\n");
        goto error;
    }

    if(fread(map->entities, sizeof(Entity), MAX_ENTITIES_PER_MAP, fp) != MAX_ENTITIES_PER_MAP) {
        fprintf(stderr, "readMap: couldn't read entities\n");
        goto error;
    }

    if(fread(&map->entityCnt, 2, 1, fp) != 1) {
        fprintf(stderr, "readMap: couldn't read entity count\n");
        goto error;
    }

    if(map->entityCnt > MAX_ENTITIES_PER_MAP) {
        fprintf(stderr, "readMap: entity count %d too large\n", map->entityCnt);
        goto error;
    }

    return 1;
error:
    return 0;
}

int mapFindEntity(const Map *map, int row, int col) {
    int i;
    const Entity *entity = &map->entities[0];

    for(i = 0; i < map->entityCnt; i++) {
        if(entity->row == row && entity->col == col) {
            return i + 1;
        }
        entity++;
    }

    return 0;
}
