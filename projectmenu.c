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
    { { "New",  "N", FALSE, projectNewClicked },
    END_SECTION };

static MenuSectionSpec openSection =
    { { "Open", "O", FALSE, projectOpenClicked },
    END_SECTION };

static MenuSectionSpec saveSection =
    { { "Save",       "S",  FALSE, projectSaveClicked   },
      { "Save As...", "A",  FALSE, projectSaveAsClicked },
      { "Revert",     NULL, TRUE,  projectRevertClicked },
    END_SECTION };

static MenuSectionSpec tilesetSection =
    { { "Select Tileset Package...", NULL, FALSE, projectSelectTilesetPackageClicked },
    END_SECTION };

static MenuSectionSpec quitSection =
    { { "Quit", "Q", FALSE, projectQuitClicked },
    END_SECTION };

MenuSectionSpec *projectMenuSpec[] = {
    &newSection,
    &openSection,
    &saveSection,
    &tilesetSection,
    &quitSection,
    END_MENU
};
