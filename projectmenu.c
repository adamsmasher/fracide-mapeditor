#include "projectmenu.h"

#include "framework/menubuild.h"

#include "ProjectWindow.h"

static MenuSectionSpec newSection = 
  { { "New",  "N", MENU_ITEM_ENABLED, newProject },
  END_SECTION };

static MenuSectionSpec openSection =
  { { "Open", "O", MENU_ITEM_ENABLED, openProject },
  END_SECTION };

static MenuSectionSpec saveSection =
  { { "Save",       "S",         MENU_ITEM_ENABLED,  saveProject   },
    { "Save As...", "A",         MENU_ITEM_ENABLED,  saveProjectAs },
    { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED, revertProject },
  END_SECTION };

static MenuSectionSpec tilesetSection =
  { { "Select Tileset Package...", NO_SHORTKEY, MENU_ITEM_ENABLED, selectTilesetPackage },
  END_SECTION };

static MenuSectionSpec quitSection =
  { { "Quit", "Q", MENU_ITEM_ENABLED, quit },
  END_SECTION };

MenuSectionSpec *projectMenuSpec[] = {
  &newSection,
  &openSection,
  &saveSection,
  &tilesetSection,
  &quitSection,
  END_MENU
};
