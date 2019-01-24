#include "project.h"

#include <proto/exec.h>

#include "map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION 1
#define HEADER (('F' << 24) | ('R' << 16) | ('M' << 8) | 'P')

static void initMaps(Project *project) {
  int i;

  project->mapCnt = 0;
  for(i = 0; i < MAX_MAPS_IN_PROJECT; i++) {
    project->maps[i] = NULL;
  }
}

static void initEntities(Project *project) {
  int i;
  for(i = 0; i < MAX_ENTITIES_IN_PROJECT; i++) {
    project->entityNameStrs[i][0] = '\0';
  }
}

static void initSongs(Project *project) {
  int i = 0;
  for(i = 0; i < MAX_SONGS_IN_PROJECT; i++) {
    project->songNameStrs[i][0] = '\0';
  }
}

void initProject(Project *project) {
  project->tilesetPackagePath[0] = '\0';
  initMaps(project);
  initEntities(project);
  initSongs(project);
}

void freeProject(Project *project) {
  int i;

  for(i = 0; i < MAX_MAPS_IN_PROJECT; i++) {
    if(project->maps[i]) {
      free(project->maps[i]);
    }
  }
}

void copyProject(Project *src, Project *dest) {
  memcpy(dest, src, sizeof(Project));
}

BOOL loadProjectFromFile(FILE *fp, Project *project) {
  int i;
  ULONG header;
  UWORD version;

  initProject(project);

  if(fread(&header, 4, 1, fp) != 1) {
    fprintf(stderr, "loadProjectFromFile: couldn't read header\n");
    return FALSE;
  }

  if(header != HEADER) {
    fprintf(stderr, "loadProjectFromFile: Invalid header value\n");
    return FALSE;
  }

  if(fread(&version, 2, 1, fp) != 1) {
    fprintf(stderr, "loadProjectFromFile: Error loading version\n");
    return FALSE;
  }

  if(version != VERSION) {
    fprintf(stderr, "loadProjectFromFile: Invalid version\n");
    return FALSE;
  }

  if(fread(project->tilesetPackagePath, 1, 256, fp) != 256) {
    fprintf(stderr, "loadProjectFromFile: Error loading package path\n");
    return FALSE;
  }

  if(fread(&project->mapCnt, 2, 1, fp) != 1) {
    fprintf(stderr, "loadProjectFromFile: Error loading mapcnt\n");
    return FALSE;
  }

  /* skip the index, we don't need it */
  fseek(fp, 256, SEEK_CUR);

  for(i = 0; i < project->mapCnt; i++) {
    UWORD mapNum;
    Map *map;

    if(fread(&mapNum, 2, 1, fp) != 1) {
      fprintf(stderr, "loadProjectFromFile: couldn't read map number\n");
      goto freeMaps_error;
    }

    if(mapNum >= 128) {
      fprintf(stderr, "loadProjectFromFile: invalid map number\n");
      goto freeMaps_error;
    }

    map = malloc(sizeof(Map));
    if(!map) {
      fprintf(stderr, "loadProjectFromFile: couldn't allocate map\n");
      goto freeMaps_error;
    }

    if(!readMap(map, fp)) {
      fprintf(stderr, "loadProjectFromFile: couldn't read map\n");
      free(map);
      goto freeMaps_error;
    }

    project->maps[mapNum] = map;
  }

  if(fread(project->songNameStrs, 80, 128, fp) != 128) {
    fprintf(stderr, "loadProjectFromFile: couldn't read song names\n");
    goto freeMaps_error;
  }

  if(fread(project->entityNameStrs, 80, 128, fp) != 128) {
    fprintf(stderr, "loadProjectFromFile: couldn't read entity names\n");
    goto freeMaps_error;
  }

  return TRUE;

freeMaps_error:
  for(i = 0; i < 128; i++) {
    free(project->maps[i]);
  }
  return FALSE;
}

static void writeIndexToFp(Project *project, FILE *fp) {
    int i;
    UWORD zero = 0;
    UWORD cnt =  1;

    for(i = 0; i < 128; i++) {
        if(project->maps[i]) {
            fwrite(&cnt, 2, 1, fp);
            cnt++;
        } else {
            fwrite(&zero, 2, 1, fp);
        }
    }
}

void saveProjectToFile(Project *project, FILE *fp) {
  ULONG header;
  UWORD version;
  UWORD i;

  header = HEADER;
  fwrite(&header, 4, 1, fp);

  version = VERSION;
  fwrite(&version, 2, 1, fp);

  fwrite(project->tilesetPackagePath, 1, 256, fp);

  fwrite(&project->mapCnt, 2, 1, fp);

  writeIndexToFp(project, fp);

  for(i = 0; i < project->mapCnt; i++) {
      if(project->maps[i] != NULL) {
        fwrite(&i, 2, 1, fp);
        writeMap(project->maps[i], fp);
      }
  }

  fwrite(project->songNameStrs, 80, 128, fp);
  fwrite(project->entityNameStrs, 80, 128, fp);
}
