#include "musicmenu.h"

#include "menubuild.h"

/* TODO */
static void songNamesClicked(void) {
}

static MenuSectionSpec musicSection =
    { { "Song Names...", NULL, FALSE, songNamesClicked },
    END_SECTION };

MenuSectionSpec *musicMenuSpec[] = {
    &musicSection,
    END_MENU
};