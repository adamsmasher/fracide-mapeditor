#include "entitiesmenu.h"

#include "framework/menubuild.h"
#include "ProjectWindow.h"

static void entityEditorClicked(FrameworkWindow *window) {
  projectWindowShowEntityNamesEditor(window);
}

static MenuSectionSpec entitySection =
  { { "Entity Editor...", NO_SHORTKEY, MENU_ITEM_ENABLED, entityEditorClicked },
  END_SECTION
};

MenuSectionSpec *entitiesMenuSpec[] = {
  &entitySection,
  END_MENU
};
