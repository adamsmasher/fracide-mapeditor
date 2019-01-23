#include "projectmenu.h"

#include "framework/menubuild.h"

#include "ProjectWindow.h"

static MenuSectionSpec newSection = 
  { { "New",  "N", MENU_ITEM_ENABLED, projectWindowNewProject },
  END_SECTION };

static MenuSectionSpec openSection =
  { { "Open", "O", MENU_ITEM_ENABLED, projectWindowOpenProject },
  END_SECTION };

static MenuSectionSpec saveSection =
  { { "Save",       "S",         MENU_ITEM_ENABLED,  (Handler)projectWindowSaveProject   },
    { "Save As...", "A",         MENU_ITEM_ENABLED,  (Handler)projectWindowSaveProjectAs },
    { "Export...",  NO_SHORTKEY, MENU_ITEM_ENABLED,  projectWindowExport },
    { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED, projectWindowRevertProject },
  END_SECTION };

static MenuSectionSpec tilesetSection =
  { { "Select Tileset Package...", NO_SHORTKEY, MENU_ITEM_ENABLED, projectWindowSelectTilesetPackage },
  END_SECTION };

static MenuSectionSpec quitSection =
  { { "Quit", "Q", MENU_ITEM_ENABLED, projectWindowQuit },
  END_SECTION };

MenuSectionSpec *projectMenuSpec[] = {
  &newSection,
  &openSection,
  &saveSection,
  &tilesetSection,
  &quitSection,
  END_MENU
};
