#include "Map.h"

#include <stdlib.h>
#include <string.h>

Map *allocMap(void) {
	Map *map = malloc(sizeof(Map));
	map->name[0] = '\0';
	map->tilesetNum = 0;
	memset(map->tiles, 0, MAP_TILES_WIDE * MAP_TILES_HIGH);
	return map;
}
