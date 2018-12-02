#include "musicmenu.h"

#include "framework/menubuild.h"
#include "ProjectWindow.h"

static void songNamesClicked(FrameworkWindow *projectWindow) {
  projectWindowShowSongNamesEditor(projectWindow);
}

static MenuSectionSpec musicSection =
    { { "Song Names...", NO_SHORTKEY, MENU_ITEM_ENABLED, songNamesClicked },
    END_SECTION };

MenuSectionSpec *musicMenuSpec[] = {
    &musicSection,
    END_MENU
};
