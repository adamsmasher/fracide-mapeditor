#include "TilesetPackage.h"

#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

#define HEADER_LENGTH 4
static char correctHeader[HEADER_LENGTH] = {'F', 'R', 'A', 'C'};

TilesetPackage *tilesetPackageLoadFromFile(char *file) {
	TilesetPackage *tilesetPackage;
	BYTE header[HEADER_LENGTH];
	BPTR fp = Open(file, MODE_OLDFILE);
	if(!fp) {
		goto done;
	}
	
	if(Read(fp, &header, HEADER_LENGTH) != 4) {
		goto closeFile;
	}
	if(memcmp(correctHeader, header, HEADER_LENGTH) != 0) {
		goto closeFile;
	}

	tilesetPackage = malloc(sizeof(TilesetPackage));
	if(!tilesetPackage) {
		goto closeFile;
	}

	Read(fp, tilesetPackage, sizeof(TilesetPackage));

closeFile:	
	Close(fp);
done:
	return tilesetPackage;
}
