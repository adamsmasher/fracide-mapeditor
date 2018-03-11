#include "mapmenu.h"

#include "framework/menubuild.h"

#include "ProjectWindow.h"

static MenuSectionSpec mapSection =
    { { "New Map",     NO_SHORTKEY, MENU_ITEM_ENABLED, newMap  },
      { "Open Map...", NO_SHORTKEY, MENU_ITEM_ENABLED, openMap },
    END_SECTION };

MenuSectionSpec *mapMenuSpec[] = {
    &mapSection,
    END_MENU
};
