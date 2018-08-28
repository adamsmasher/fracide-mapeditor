#include "mapmenu.h"

#include "framework/menubuild.h"

#include "ProjectWindow.h"

static MenuSectionSpec mapSection =
    { { "New Map",     NO_SHORTKEY, MENU_ITEM_ENABLED, projectWindowNewMap  },
      { "Open Map...", NO_SHORTKEY, MENU_ITEM_ENABLED, projectWindowOpenMap },
    END_SECTION };

MenuSectionSpec *mapMenuSpec[] = {
    &mapSection,
    END_MENU
};
