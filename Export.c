#include "Export.h"

#include <string.h>

#include "Compressor.h"

static Map emptyMap;

typedef struct MapIndex_tag {
  UWORD mapPointers[128];
} MapIndex;

void writeDummyHeader(FILE *fp) {
  int i;
  for(i = 0; i < sizeof(MapIndex); i++) {
    fputc(0, fp);
  }
}

void writeMapIndex(MapIndex *mapIndex, FILE *fp) {
  fwrite(mapIndex, sizeof(MapIndex), 1, fp);
}

int writeCompressedMap(Map *map, FILE *fp) {
  return compress((UBYTE*)&map->tiles, MAP_TILES_WIDE * MAP_TILES_HIGH, fp);
}

BOOL exportProject(Project *project, FILE *fp) {
  int i;
  MapIndex mapIndex;
  UWORD offset = 0x4000;

  memset(emptyMap.tiles, 0, MAP_TILES_WIDE * MAP_TILES_HIGH);

  writeDummyHeader(fp);
  for(i = 0; i < MAX_MAPS_IN_PROJECT; i++) {
    Map *map = project->maps[i] ? project->maps[i] : &emptyMap;
    mapIndex.mapPointers[i] = offset;
    offset += writeCompressedMap(map, fp);
  }
  rewind(fp);
  writeMapIndex(&mapIndex, fp);

  return TRUE;
}
