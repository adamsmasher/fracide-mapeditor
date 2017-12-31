#include "projectmenu.h"

#include "currentproject.h"
#include "menubuild.h"
#include "workspace.h"

static void projectNewClicked(void) {
    newProject();
}

static void projectOpenClicked(void) {
    openProject();
}

static void projectSaveClicked(void) {
    saveProject();
}

static void projectSaveAsClicked(void) {
    saveProjectAs();
}

static void projectRevertClicked(void) {
    revertProject();
}

static void projectSelectTilesetPackageClicked(void) {
    selectTilesetPackage();
}

static void projectQuitClicked(void) {
    quit();
}

static MenuSectionSpec newSection = 
    { { "New",  "N", MENU_ITEM_ENABLED, projectNewClicked },
    END_SECTION };

static MenuSectionSpec openSection =
    { { "Open", "O", MENU_ITEM_ENABLED, projectOpenClicked },
    END_SECTION };

static MenuSectionSpec saveSection =
    { { "Save",       "S",         MENU_ITEM_ENABLED,  projectSaveClicked   },
      { "Save As...", "A",         MENU_ITEM_ENABLED,  projectSaveAsClicked },
      { "Revert",     NO_SHORTKEY, MENU_ITEM_DISABLED, projectRevertClicked },
    END_SECTION };

static MenuSectionSpec tilesetSection =
    { { "Select Tileset Package...", NO_SHORTKEY, MENU_ITEM_ENABLED, projectSelectTilesetPackageClicked },
    END_SECTION };

static MenuSectionSpec quitSection =
    { { "Quit", "Q", MENU_ITEM_ENABLED, projectQuitClicked },
    END_SECTION };

MenuSectionSpec *projectMenuSpec[] = {
    &newSection,
    &openSection,
    &saveSection,
    &tilesetSection,
    &quitSection,
    END_MENU
};
