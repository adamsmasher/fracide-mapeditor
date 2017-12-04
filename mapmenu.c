#include "mapmenu.h"

#include "menubuild.h"

static void mapNewClicked(void) {
    newMap();
}

static void mapOpenClicked(void) {
    openMap();
}

/* TODO: add a constant/enum for NO_SHORTKEY, ENABLED */
static MenuSectionSpec mapSection =
    { { "New Map",     NULL, FALSE, mapNewClicked  },
      { "Open Map...", NULL, FALSE, mapOpenClicked },
    END_SECTION };

MenuSectionSpec *mapMenuSpec[] = {
    &mapSection,
    END_MENU
};
