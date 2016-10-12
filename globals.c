#include <intuition/intuition.h>

#include "globals.h"

#include "TilesetPackage.h"
#include "Project.h"

struct Screen   *screen          = NULL;
void            *vi              = NULL;
TilesetPackage  *tilesetPackage  = NULL;
struct Window   *projectWindow   = NULL;
SongRequester   *songNamesEditor = NULL;
Project         project;
int             projectSaved     = 1;
char            projectFilename[PROJECT_FILENAME_LENGTH];
struct TextAttr Topaz80 = { "topaz.font", 8, 0, 0 };
