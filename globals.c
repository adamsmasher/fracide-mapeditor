#include <intuition/intuition.h>

#include "globals.h"

#include "TilesetPackage.h"
#include "Project.h"

struct Screen  *screen         = NULL;
void           *vi             = NULL;
TilesetPackage *tilesetPackage = NULL;
struct Window  *projectWindow  = NULL;
Project        project;
char           projectFilename[PROJECT_FILENAME_LENGTH];
