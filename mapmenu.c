#include "mapmenu.h"

#include "framework/menubuild.h"
#include "workspace.h"

static void mapNewClicked(void) {
    newMap();
}

static void mapOpenClicked(void) {
    openMap();
}

static MenuSectionSpec mapSection =
    { { "New Map",     NO_SHORTKEY, MENU_ITEM_ENABLED, mapNewClicked  },
      { "Open Map...", NO_SHORTKEY, MENU_ITEM_ENABLED, mapOpenClicked },
    END_SECTION };

MenuSectionSpec *mapMenuSpec[] = {
    &mapSection,
    END_MENU
};
