#ifndef MENU_H
#define MENU_H

#include <libraries/gadtools.h>

#include "framework/menubuild.h"

#define REVERT_PROJECT_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))
#define REVERT_MAP_MENU_ITEM     (SHIFTMENU(0) | SHIFTITEM(6))

extern MenuSpec *mainMenuSpec;

#endif
