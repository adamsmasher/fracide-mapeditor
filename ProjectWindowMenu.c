#include "ProjectWindowMenu.h"

#include <proto/intuition.h>

#include "framework/menubuild.h"

#include "EntitiesMenu.h"
#include "MapMenu.h"
#include "MusicMenu.h"
#include "ProjectMenu.h"

static MenuSpec specs[] = {
  { "Project",  &projectMenuSpec  },
  { "Maps",     &mapMenuSpec      },
  { "Entities", &entitiesMenuSpec },
  { "Music",    &musicMenuSpec    },
  END_MENUS
};

MenuSpec *projectWindowMenuSpec = specs;

#define REVERT_PROJECT_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

void projectWindowMenuEnableRevertProject(FrameworkWindow *window) {
  OnMenu(window->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
}

void projectWindowMenuDisableRevertProject(FrameworkWindow *window) {
  OffMenu(window->intuitionWindow, REVERT_PROJECT_MENU_ITEM);
}
