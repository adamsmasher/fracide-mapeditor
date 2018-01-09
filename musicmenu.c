#include "musicmenu.h"

#include "framework/menubuild.h"
#include "SongNamesEditor.h"

static void songNamesClicked(void) {
    showSongNamesEditor();
}

static MenuSectionSpec musicSection =
    { { "Song Names...", NO_SHORTKEY, MENU_ITEM_ENABLED, songNamesClicked },
    END_SECTION };

MenuSectionSpec *musicMenuSpec[] = {
    &musicSection,
    END_MENU
};
