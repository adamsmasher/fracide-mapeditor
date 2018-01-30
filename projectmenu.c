#include "projectmenu.h"

#include "framework/menubuild.h"

#include "currentproject.h"
#include "workspace.h"

static void projectNewClicked(FrameworkWindow *window) {
    newProject();
}

static void projectOpenClicked(FrameworkWindow *window) {
    openProject();
}

static void projectSaveClicked(FrameworkWindow *window) {
    saveProject();
}

static void projectSaveAsClicked(FrameworkWindow *window) {
    saveProjectAs();
}

static void projectRevertClicked(FrameworkWindow *window) {
    revertProject();
}

static void projectSelectTilesetPackageClicked(FrameworkWindow *window) {
    selectTilesetPackage();
}

static void projectQuitClicked(FrameworkWindow *window) {
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
