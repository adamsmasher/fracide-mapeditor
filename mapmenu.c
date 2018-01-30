#include "mapmenu.h"

#include "framework/menubuild.h"
#include "workspace.h"

static void mapNewClicked(FrameworkWindow *window) {
    newMap();
}

static void mapOpenClicked(FrameworkWindow *window) {
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
