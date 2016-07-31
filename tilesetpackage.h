#ifndef TILESET_PACKAGE_H
#define TILESET_PACKAGE_H

#include <exec/types.h>

#define TILESETS      16
#define TILES_PER_SET 64

typedef struct TilesetPackageTag {
	WORD  tilesetCnt;
	char  tilesetNames[TILESETS][16];
	UBYTE tilesetImgs[TILESETS][TILES_PER_SET][64];
	BOOL  passable[TILESETS][TILES_PER_SET];	
} TilesetPackage;

TilesetPackage *tilesetPackageLoadFromFile(char *);

#endif
