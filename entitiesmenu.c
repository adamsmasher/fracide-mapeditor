#include "entitiesmenu.h"

#include "menubuild.h"
#include "workspace.h"

static void entityEditorClicked(void) {
    showEntityEditor();
}

static MenuSectionSpec entitySection =
    { { "Entity Editor...", 0, FALSE, entityEditorClicked },
    END_SECTION };

MenuSectionSpec *entitiesMenuSpec[] = {
    &entitySection,
    END_MENU
};
