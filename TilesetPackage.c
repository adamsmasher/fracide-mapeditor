#include "TilesetPackage.h"

#include <proto/dos.h>
#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

#define HEADER_LENGTH 4
static char correctHeader[HEADER_LENGTH] = {'F', 'R', 'A', 'C'};

static int loadFile(BPTR fp, TilesetPackageFile *file) {
	char header[HEADER_LENGTH];
	UBYTE *buffer;
	long bytesRead;
	long i;
	long bytesToRead;

	if(Read(fp, header, (long)HEADER_LENGTH) != 4L) {
		goto error;
	}
	if(memcmp(correctHeader, header, HEADER_LENGTH)) {
		goto error;
	}

	buffer = (UBYTE*)file;
	i = 0;
	bytesRead = 0;
	bytesToRead = sizeof(TilesetPackageFile);
	while(bytesToRead > 0L && (bytesRead = Read(fp, &buffer[i], bytesToRead))) {
		if(bytesRead == -1L) {
			goto error;
		}
		i += bytesRead;
		bytesToRead -= bytesRead;
	}
	if(bytesToRead > 0L) {
		goto error;
	}

	return 1;
error:
	return 0;
}

static void freeNames(struct List *tilesetNames) {
	struct Node *node, *next;
	node = tilesetNames->lh_Head;
	while(next = node->ln_Succ) {
		free(node);
		node = next;
	}
}

static int initTilesetNames(TilesetPackage *tilesetPackage) {
	WORD i;
	struct Node *node;

	NewList(&tilesetPackage->tilesetNames);

	for(i = 0; i < tilesetPackage->tilesetPackageFile.tilesetCnt; i++) {
		node = malloc(sizeof(struct Node));
		if(!node) {
			goto error;
		}
		node->ln_Name = &(tilesetPackage->tilesetPackageFile.tilesetNames[i][0]);
		AddTail(&tilesetPackage->tilesetNames, node);
	}

	return 1;
error:
	freeNames(&tilesetPackage->tilesetNames);
	return 0;
}

TilesetPackage *tilesetPackageLoadFromFp(BPTR fp) {
	TilesetPackage *tilesetPackage = malloc(sizeof(TilesetPackage));
	if(!tilesetPackage) {
		goto error;
	}

	if(!loadFile(fp, &tilesetPackage->tilesetPackageFile)) {
		goto error_freeTilesetPackage;
	}

	if(!initTilesetNames(tilesetPackage)) {
		goto error_freeTilesetPackage;
	}
done:
	return tilesetPackage;
	
error_freeTilesetPackage:
	free(tilesetPackage);
error:
	return NULL;
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
	return tilesetPackage;
}

void freeTilesetPackage(TilesetPackage *tilesetPackage) {
	if(tilesetPackage) {
		freeNames(&tilesetPackage->tilesetNames);
		free(tilesetPackage);
	}
}
