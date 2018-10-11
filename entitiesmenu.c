#include "entitiesmenu.h"

#include "framework/menubuild.h"
#include "EntityNamesEditor.h"

static void entityEditorClicked(FrameworkWindow *window) {
  showEntityNamesEditor(window);
}

static MenuSectionSpec entitySection =
    { { "Entity Editor...", NO_SHORTKEY, MENU_ITEM_ENABLED, entityEditorClicked },
    END_SECTION };

MenuSectionSpec *entitiesMenuSpec[] = {
    &entitySection,
    END_MENU
};
