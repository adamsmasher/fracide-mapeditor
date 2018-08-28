#include "MapEditorMenu.h"

#include <proto/intuition.h>

#include "MapEditor.h"

static MenuSectionSpec newSection =
  { { "New", "N", MENU_ITEM_ENABLED, mapEditorNewMap },
    END_SECTION };

static MenuSectionSpec openSection =
  { { "Open", "O", MENU_ITEM_ENABLED, mapEditorOpenMap },
    END_SECTION };

static MenuSectionSpec saveSection =
  { { "Save",       "S",         MENU_ITEM_ENABLED,  (Handler)mapEditorSaveMap   },
    { "Save As...", "A",         MENU_ITEM_ENABLED,  (Handler)mapEditorSaveMapAs },
    { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED,          mapEditorRevertMap },
  END_SECTION };

static MenuSectionSpec closeSection =
  { { "Close", "Q", MENU_ITEM_ENABLED, (Handler)tryToCloseWindow },
    END_SECTION };

static MenuSectionSpec *mapMenuSpec[] = {
  &newSection,
  &openSection,
  &saveSection,
  &closeSection,
  END_MENU
};

#define REVERT_MAP_MENU_ITEM (SHIFTMENU(0) | SHIFTITEM(6))

static MenuSpec spec[] = {
  { "Map", &mapMenuSpec },
  END_MENUS
};

MenuSpec *mapEditorMenuSpec = spec;

void mapEditorMenuEnableRevertMap(FrameworkWindow *mapEditor) {
  OnMenu(mapEditor->intuitionWindow, REVERT_MAP_MENU_ITEM);
}

void mapEditorMenuDisableRevertMap(FrameworkWindow *mapEditor) {
  OffMenu(mapEditor->intuitionWindow, REVERT_MAP_MENU_ITEM);
}
