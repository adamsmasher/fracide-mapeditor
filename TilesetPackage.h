#ifndef TILESET_PACKAGE_H
#define TILESET_PACKAGE_H

#include <exec/types.h>
#include <exec/lists.h>

#define TILESETS       16
#define TILES_PER_SET  64
#define BYTES_PER_TILE 64

typedef UBYTE TilesetImgs[TILES_PER_SET][BYTES_PER_TILE];

typedef struct TilesetPackageFileTag {
	WORD        tilesetCnt;
	char        tilesetNames[TILESETS][16];
	TilesetImgs tilesetImgs[TILESETS];
	UBYTE       passable[TILESETS][TILES_PER_SET];
} TilesetPackageFile;

typedef struct TilesetPackageTag {
	TilesetPackageFile tilesetPackageFile;
	struct List tilesetNames;
} TilesetPackage;

TilesetPackage *tilesetPackageLoadFromFile(char *);
void freeTilesetPackage(TilesetPackage *);

#endif
