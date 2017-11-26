#include "mapmenu.h"

#include "menubuild.h"

/* TODO */
static void mapNewClicked(void) {
}

static void mapOpenClicked(void) {
}

/* TODO: add a constant/enum for NO_SHORTKEY, ENABLED */
static MenuSectionSpec mapSection =
    { { "New Map",     0, FALSE, mapNewClicked  },
      { "Open Map...", 0, FALSE, mapOpenClicked },
    END_SECTION };

MenuSectionSpec *mapMenuSpec[] = {
    &mapSection,
    END_MENU
};
