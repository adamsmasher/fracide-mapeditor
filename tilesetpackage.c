#include "TilesetPackage.h"

#include <proto/dos.h>

#include <stdlib.h>
#include <string.h>

#define HEADER_LENGTH 4
static char correctHeader[HEADER_LENGTH] = {'F', 'R', 'A', 'C'};

TilesetPackage *tilesetPackageLoadFromFp(BPTR fp) {
	UBYTE *tilesetPackage;
	char header[HEADER_LENGTH];
	long bytesRead;
	long bytesToRead;
	
	tilesetPackage = NULL;
	
	if(Read(fp, &header, (long)HEADER_LENGTH) != 4L) {
		goto done;
	}
	if(memcmp(correctHeader, header, HEADER_LENGTH)) {
		goto done;
	}

	tilesetPackage = malloc(sizeof(TilesetPackage));
	if(!tilesetPackage) {
		goto done;
	}

	bytesRead = 0;
	bytesToRead = sizeof(TilesetPackage);
	while(bytesToRead > 0 && (bytesRead = Read(fp, &tilesetPackage[bytesRead], bytesToRead))) {
		if(bytesRead == -1L) {
			free(tilesetPackage);
			tilesetPackage = NULL;
			goto done;
		}
		bytesToRead -= bytesRead;
	}
	if(bytesToRead > 0) {
		free(tilesetPackage);
		tilesetPackage = NULL;
		goto done;
	}

done:
	return (TilesetPackage*)tilesetPackage;
}

TilesetPackage *tilesetPackageLoadFromFile(char *file) {
	BPTR fp;
	TilesetPackage *tilesetPackage;

	fp = Open(file, MODE_OLDFILE);
	if(!fp) {
		goto done;
	}

	tilesetPackage = tilesetPackageLoadFromFp(fp);

closeFile:
	Close(fp);
done:
	return (TilesetPackage*)tilesetPackage;
}