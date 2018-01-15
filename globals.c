#include <intuition/intuition.h>

#include "globals.h"

#include "TilesetPackage.h"

struct Screen   *screen          = NULL;
void            *vi              = NULL;
TilesetPackage  *tilesetPackage  = NULL;

struct TextAttr Topaz80 = { "topaz.font", 8, 0, 0 };
