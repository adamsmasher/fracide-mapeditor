#include "project.h"

#include <stdio.h>
#include <stdlib.h>

#define VERSION 1
#define HEADER (('F' << 24) | ('R' << 16) | ('M' << 8) | 'P')

void initProject(Project *project) {
	int i;

	project->tilesetPackagePath[0] = '\0';
	for(i = 0; i < 128; i++) {
		project->maps[i] = NULL;
	}
}

static int loadProjectFromFp(FILE *fp, Project *project) {
	int i;
	long header;
	UWORD version;
	UWORD mapCnt;

	initProject(project);

	if(fread(&header, 4, 1, fp) != 4) {
		fprintf(stderr, "loadProjectFromFp: couldn't read header\n");
		return 0;
	}

	if(header != HEADER) {
		fprintf(stderr, "loadProjectFromFp: Invalid header value\n");
		return 0;
	}

	if(fread(&version, 2, 1, fp) != 2) {
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

	if(fread(&mapCnt, 2, 1, fp) != 2) {
		fprintf(stderr, "loadProjectFromFp: Error loading mapcnt\n");
		return 0;
	}

	/* skip the index, we don't need it */
	fseek(fp, 256, SEEK_CUR);

	for(i = 0; i < mapCnt; i++) {
		UWORD mapNum;
		Map *map;

		fread(&mapNum, 2, 1, fp);

		if(mapNum >= 128) {
			fprintf(stderr, "loadProjectFromFp: invalid map number\n");
			goto freeMaps_error;
		}

		map = malloc(sizeof(Map));
		if(!map) {
			fprintf(stderr, "loadProjectFromFp: couldn't allocate map\n");
			goto freeMaps_error;
		}

		if(fread(map, sizeof(Map), 1, fp) != sizeof(Map)) {
			fprintf(stderr, "loadProjectFromFp: couldn't read map\n");
			goto freeMaps_error;
		}

		project->maps[mapNum] = map;
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
