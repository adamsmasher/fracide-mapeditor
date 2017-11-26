#include "projectmenu.h"

#include "menubuild.h"

/* TODO */
static void projectNewClicked(void) {
}

static void projectOpenClicked(void) {
}

static void projectSaveClicked(void) {
}

static void projectSaveAsClicked(void) {
}

static void projectRevertClicked(void) {
}

static void projectSelectTilesetPackageClicked(void) {
}

static void projectQuitClicked(void) {
}

static MenuSectionSpec newSection = 
    { { "New",  'N', FALSE, projectNewClicked },
    END_SECTION };

static MenuSectionSpec openSection =
    { { "Open", 'O', FALSE, projectOpenClicked }, 
    END_SECTION };

static MenuSectionSpec saveSection =
    { { "Save",       'S', FALSE, projectSaveClicked   },
      { "Save As...", 'A', FALSE, projectSaveAsClicked },
      { "Revert",      0 , TRUE,  projectRevertClicked },
    END_SECTION };

static MenuSectionSpec tilesetSection =
    { { "Select Tileset Package...", 0 , FALSE, projectSelectTilesetPackageClicked },
    END_SECTION };

static MenuSectionSpec quitSection =
    { { "Quit", 'Q', FALSE, projectQuitClicked },
    END_SECTION };

MenuSectionSpec *projectMenuSpec[] = {
    &newSection,
    &openSection,
    &saveSection,
    &tilesetSection,
    &quitSection,
    END_MENU
};
