#ifndef GLOBALS_H
#define GLOBALS_H

#include <intuition/intuition.h>

#include "TilesetPackage.h"
#include "Project.h"

#define PROJECT_FILENAME_LENGTH 256

extern struct Screen  *screen;
extern void           *vi;
extern TilesetPackage *tilesetPackage;
extern struct Window  *projectWindow;
extern Project        project;
extern char           projectFilename[PROJECT_FILENAME_LENGTH];

#endif
