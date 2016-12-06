#include "project.h"

#include <proto/exec.h>

#include "globals.h"
#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION 1
#define HEADER (('F' << 24) | ('R' << 16) | ('M' << 8) | 'P')

void updateProjectMapName(Project *project, int mapNum, Map *map) {
    sprintf(project->mapNameStrs[mapNum], "%d: %s", mapNum, map->name);
}

static void initMapNameNodes(Project *project) {
    int i;
    struct Node *node, *next;

    node = project->mapNames.lh_Head;
    i = 0;
    while(next = node->ln_Succ) {
        node->ln_Name = project->mapNameStrs[i];
        node = next;
        i++;
    }
}

static void initSongNameNodes(Project *project) {
    int i;
    struct Node *node, *next;

    node = project->songNames.lh_Head;
    i = 0;
    while(next = node->ln_Succ) {
        node->ln_Name = project->songNameStrs[i];
        node = next;
        i++;
    }
}

void initProject(Project *project) {
    int i;
    struct Node *node;

    project->tilesetPackagePath[0] = '\0';

    project->mapCnt = 0;

    for(i = 0; i < 128; i++) {
        project->maps[i] = NULL;
        sprintf(project->mapNameStrs[i], "%d:", i);
    }

    NewList(&project->mapNames);

    for(i = 0; i < 128; i++) {
        node = malloc(sizeof(struct Node));
        /* TODO: handle node creation failure */
        AddTail(&project->mapNames, node);
    }

    initMapNameNodes(project);

    NewList(&project->songNames);

    for(i = 0; i < 128; i++) {
        sprintf(project->songNameStrs[i], "%d:", i);
        node = malloc(sizeof(struct Node));
        /* TODO: handle node creation failure */
        AddTail(&project->songNames, node);
    }

    initSongNameNodes(project);
}

void freeProject(Project *project) {
    int i;
    struct Node *node, *next;

    for(i = 0; i < 128; i++) {
        if(project->maps[i]) {
            freeMap(project->maps[i]);
            free(project->maps[i]);
        }
    }

    node = project->mapNames.lh_Head;
    while(next = node->ln_Succ) {
        free(node);
        node = next;
    }

    node = project->songNames.lh_Head;
    while(next = node->ln_Succ) {
        free(node);
        node = next;
    }
}

void copyProject(Project *src, Project *dest) {
    memcpy(dest, src, sizeof(Project));
    /* fix up internal pointers */
    initMapNameNodes(dest);
    initSongNameNodes(dest);
}

static int loadProjectFromFp(FILE *fp, Project *project) {
    int i;
    ULONG header;
    UWORD version;

    initProject(project);

    if(fread(&header, 4, 1, fp) != 1) {
        fprintf(stderr, "loadProjectFromFp: couldn't read header\n");
        return 0;
    }

    if(header != HEADER) {
        fprintf(stderr, "loadProjectFromFp: Invalid header value\n");
        return 0;
    }

    if(fread(&version, 2, 1, fp) != 1) {
        fprintf(stderr, "loadProjectFromFp: Error loading version\n");
        return 0;
    }

    if(version != VERSION) {
        fprintf(stderr, "loadProjectFromFile: Invalid version\n");
        return 0;
    }

    if(fread(project->tilesetPackagePath, 1, 256, fp) != 256) {
        fprintf(stderr, "loadProjectFromFp: Error loading package path\n");
        return 0;
    }

    if(fread(&project->mapCnt, 2, 1, fp) != 1) {
        fprintf(stderr, "loadProjectFromFp: Error loading mapcnt\n");
        return 0;
    }

    /* skip the index, we don't need it */
    fseek(fp, 256, SEEK_CUR);

    for(i = 0; i < project->mapCnt; i++) {
        UWORD mapNum;
        Map *map;

        if(fread(&mapNum, 2, 1, fp) != 1) {
            fprintf(stderr, "loadProjectFromFp: couldn't read map number\n");
            goto freeMaps_error;
        }

        if(mapNum >= 128) {
            fprintf(stderr, "loadProjectFromFp: invalid map number\n");
            goto freeMaps_error;
        }

        map = malloc(sizeof(Map));
        if(!map) {
            fprintf(stderr, "loadProjectFromFp: couldn't allocate map\n");
            goto freeMaps_error;
        }

        if(!readMap(map, fp)) {
            fprintf(stderr, "loadProjectFromFp: couldn't read map\n");
            free(map);
            goto freeMaps_error;
        }

        project->maps[mapNum] = map;
        updateProjectMapName(project, mapNum, map);
    }

    if(fread(project->songNameStrs, 80, 128, fp) != 128) {
        fprintf(stderr, "loadProjectFromFp: couldn't read song names\n");
        goto freeMaps_error;
    }

    return 1;

freeMaps_error:
    for(i = 0; i < 128; i++) {
        free(project->maps[i]);
    }
    return 0;
}

int loadProjectFromFile(char *file, Project *project) {
    int ret;
    FILE *fp = fopen(file, "rb");

    if(!fp) {
        fprintf(stderr, "loadProjectFromFile: error loading %s\n", file);
        ret = 0;
        goto done;
    }

    ret = loadProjectFromFp(fp, project);

    fclose(fp);
done:
    return ret;
}

static void writeIndexToFp(FILE *fp) {
    int i;
    UWORD zero = 0;
    UWORD cnt =  1;

    for(i = 0; i < 128; i++) {
        if(project.maps[i]) {
            fwrite(&cnt, 2, 1, fp);
            cnt++;
        } else {
            fwrite(&zero, 2, 1, fp);
        }
    }
}

static void saveProjectToFp(FILE *fp) {
    ULONG header;
    UWORD version;
    UWORD i;

    header = HEADER;
    fwrite(&header, 4, 1, fp);

    version = VERSION;
    fwrite(&version, 2, 1, fp);

    fwrite(project.tilesetPackagePath, 1, 256, fp);

    fwrite(&project.mapCnt, 2, 1, fp);

    writeIndexToFp(fp);

    for(i = 0; i < project.mapCnt; i++) {
        if(project.maps[i] != NULL) {
            fwrite(&i, 2, 1, fp);
            writeMap(project.maps[i], fp);
        }
    }

    fwrite(project.songNameStrs, 80, 128, fp);
}

int saveProjectToFile(char *file) {
    int ret;
    FILE *fp = fopen(file, "wb");

    if(!fp) {
        fprintf(stderr, "saveProjectToFile: error opening %s\n", file);
        ret = 0;
        goto done;
    }

    saveProjectToFp(fp);
    ret = 1;

    fclose(fp);
done:
    return ret;
}
