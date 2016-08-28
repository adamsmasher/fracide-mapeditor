#include "project.h"

#include "globals.h"

#include <stdio.h>
#include <stdlib.h>

#define VERSION 1
#define HEADER (('F' << 24) | ('R' << 16) | ('M' << 8) | 'P')

void initProject(Project *project) {
	int i;

	project->tilesetPackagePath[0] = '\0';
	project->mapCnt = 0;
	for(i = 0; i < 128; i++) {
		project->maps[i] = NULL;
	}
}

void freeProject(Project *project) {
	int i;
	for(i = 0; i < 128; i++) {
		free(project->maps[i]);
	}
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
		Map *map;

		map = malloc(sizeof(Map));
		if(!map) {
			fprintf(stderr, "loadProjectFromFp: couldn't allocate map\n");
			goto freeMaps_error;
		}

		if(fread(map, sizeof(Map), 1, fp) != sizeof(Map)) {
			fprintf(stderr, "loadProjectFromFp: couldn't read map\n");
			free(map);
			goto freeMaps_error;
		}

		if(!map->mapNum || map->mapNum > 128) {
			fprintf(stderr, "loadProjectFromFp: invalid map number\n");
			free(map);
			goto freeMaps_error;
		}

		project->maps[map->mapNum - 1] = map;
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
	int i;

	header = HEADER;
	fwrite(&header, 4, 1, fp);

	version = VERSION;
	fwrite(&version, 2, 1, fp);

	fwrite(project.tilesetPackagePath, 1, 256, fp);

	fwrite(&project.mapCnt, 2, 1, fp);

	writeIndexToFp(fp);

	for(i = 0; i < project.mapCnt; i++) {
		if(project.maps[i] != NULL) {
			fwrite(project.maps[i], sizeof(Map), 1, fp);
		}
	}
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
