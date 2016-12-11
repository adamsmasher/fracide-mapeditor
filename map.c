#include "Map.h"

#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

#define ENTITY_LABEL_LENGTH 16

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
    NewList(&map->entityLabels);
    return map;
}

static struct Node *makeEntityNode(char *label) {
    struct Node *node;

    node = malloc(sizeof(struct Node));
    if(!node) {
        fprintf(stderr, "makeEntityNode: couldn't allocate new node");
        goto error;
    }

    node->ln_Name = label;

    return node;
error:
    return NULL;

}

static struct Node *makeNumberedEntityNode(int num) {
    struct Node *node;
    char *label;

    label = malloc(ENTITY_LABEL_LENGTH);
    if(!label) {
        fprintf(stderr, "makeNumberedEntityNode: couldn't allocate new label");
        goto error;
    }
    sprintf(label, "%d: N/A", num);

    node = makeEntityNode(label);
    if(!node) {
        fprintf(stderr, "makeNumeredEntityNode: couldn't make node");
        goto error_freeLabel;
    }

    return node;
error_freeLabel:
    free(label);
error:
    return NULL;
}

static void freeEntityLabels(Map *map) {
    struct Node *node, *next;

    node = map->entityLabels.lh_Head;
    while(next = node->ln_Succ) {
        free(node->ln_Name);
        free(node);
        node = next;
    }
}

static struct Node *copyEntityNode(struct Node *node) {
    struct Node *nodeCopy;
    char *label;

    label = malloc(ENTITY_LABEL_LENGTH);
    if(!label) {
        fprintf(stderr, "copyEntityNode: couldn't allocate label");
        goto error;
    }
    strcpy(label, node->ln_Name);

    nodeCopy = makeEntityNode(label);
    if(!nodeCopy) {
        fprintf(stderr, "copyEntityNode: couldn't make node");
        goto error_freeLabel;
    }

    return nodeCopy;
error_freeLabel:
    free(label);
error:
    return NULL;
}

static int copyEntityLabels(Map *srcMap, Map *destMap) {
    struct Node *node, *next;

    NewList(&destMap->entityLabels);
    node = srcMap->entityLabels.lh_Head;
    while(next = node->ln_Succ) {
        node = copyEntityNode(node);
        if(!node) {
            fprintf(stderr, "copyEntityLabels: couldn't make entity");
            goto error;
        }
        AddTail(&destMap->entityLabels, node);
        node = next;
    }

    return 1;
error:
    freeEntityLabels(destMap);
    return 0;
}

void freeMap(Map *map) {
    freeEntityLabels(map);
}

void overwriteMap(Map *srcMap, Map *destMap) {
    freeMap(destMap);

    strcpy(destMap->name, srcMap->name);
    destMap->tilesetNum = srcMap->tilesetNum;
    destMap->songNum = srcMap->songNum;
    destMap->entityCnt = srcMap->entityCnt;
    memcpy(destMap->tiles, srcMap->tiles, MAP_TILES_WIDE * MAP_TILES_HIGH);
    memcpy(destMap->entities, srcMap->entities, sizeof(Entity) * srcMap->entityCnt);
    copyEntityLabels(srcMap, destMap);
}

Map *copyMap(Map *oldMap) {
    Map *newMap = malloc(sizeof(Map));
    if(!newMap) {
        return NULL;
    }
    overwriteMap(oldMap, newMap);

    return newMap;
}

int mapAddNewEntity(Map *map) {
    Entity *entity = &map->entities[map->entityCnt];
    struct Node *node;

    node = makeNumberedEntityNode(map->entityCnt + 1);
    if(!node) {
        fprintf(stderr, "mapAddNewEntity: couldn't create new entity\n");
        goto error;
    }

    map->entityCnt++;

    entity->entityNum = 0;
    entity->row = 0;
    entity->col = 0;
    entity->vramSlot = 0;

    AddTail(&map->entityLabels, node);

    return 1;
error:
    return 0;
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
