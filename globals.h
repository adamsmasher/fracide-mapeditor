#ifndef GLOBALS_H
#define GLOBALS_H

#include <intuition/intuition.h>

#include "EntityRequester.h"
#include "Project.h"
#include "TilesetPackage.h"

#define PROJECT_FILENAME_LENGTH 256

extern struct Screen   *screen;
extern void            *vi;
extern TilesetPackage  *tilesetPackage;
/* TODO: move me into a file */
/* TODO: rename me to entityNamesEditor */
extern EntityRequester *entityEditor;
extern int             running;
/* TODO: move the next three into current project */
extern Project         project;
extern int             projectSaved;
extern char            projectFilename[PROJECT_FILENAME_LENGTH];
extern struct TextAttr Topaz80;

#endif
