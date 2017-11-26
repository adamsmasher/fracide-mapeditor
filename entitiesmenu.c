#include "entitiesmenu.h"

#include "menubuild.h"

/* TODO */
static void entityEditorClicked(void) {
}

static MenuSectionSpec entitySection =
    { { "Entity Editor...", 0, FALSE, entityEditorClicked },
    END_SECTION };

MenuSectionSpec *entitiesMenuSpec[] = {
    &entitySection,
    END_MENU
};
