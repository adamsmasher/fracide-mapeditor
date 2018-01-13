#ifndef GLOBALS_H
#define GLOBALS_H

#include <intuition/intuition.h>

#include "Project.h"
#include "TilesetPackage.h"

extern struct Screen   *screen;
extern void            *vi;
extern TilesetPackage  *tilesetPackage;
extern int             running;
extern struct TextAttr Topaz80;

#endif
