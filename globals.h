#ifndef GLOBALS_H
#define GLOBALS_H

#include <intuition/intuition.h>

#include "EntityEditor.h"
#include "Project.h"
#include "SongNames.h"
#include "TilesetPackage.h"

#define PROJECT_FILENAME_LENGTH 256

extern struct Screen   *screen;
extern void            *vi;
extern TilesetPackage  *tilesetPackage;
extern struct Window   *projectWindow;
extern SongRequester   *songNamesEditor;
extern EntityEditor    *entityEditor;
extern Project         project;
extern int             projectSaved;
extern char            projectFilename[PROJECT_FILENAME_LENGTH];
extern struct TextAttr Topaz80;

#endif
