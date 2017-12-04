#include "musicmenu.h"

#include "menubuild.h"
#include "workspace.h"

static void songNamesClicked(void) {
    showSongNamesEditor();
}

static MenuSectionSpec musicSection =
    { { "Song Names...", NULL, FALSE, songNamesClicked },
    END_SECTION };

MenuSectionSpec *musicMenuSpec[] = {
    &musicSection,
    END_MENU
};
