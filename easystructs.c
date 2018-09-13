#include "easystructs.h"

#include <intuition/intuition.h>

/* TODO: Split this into easy structs for a particular module */

struct EasyStruct noTilesetPackageLoadedEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "No Tileset Package Loaded",
    "Cannot choose tileset when no tileset package has been loaded.",
    "Select Tileset Package...|Cancel"
};

struct EasyStruct tilesetPackageLoadFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Loading Tileset Package",
    "Could not load tileset package from\n%s.",
    "OK"
};

struct EasyStruct projectLoadFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Loading Project",
    "Could not load project from\n%s.",
    "OK"
};

struct EasyStruct projectSaveFailEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Error Saving Project",
    "Could not save project to \n%s.",
    "OK"
};

struct EasyStruct unsavedMapAlertEasyStructWithNum = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Map",
    "Save changes to map %ld, \"%s\"?",
    "Save|Don't Save|Cancel"
};

struct EasyStruct unsavedMapAlertEasyStructNoNum = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Map",
    "Save changes to \"%s\"?",
    "Save|Don't Save|Cancel"
};

struct EasyStruct saveIntoFullSlotEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Overwrite",
    "Map slot %ld is already occupied by \"%s\".\nAre you sure you want to overwrite it?",
    "Overwrite|Cancel"
};

struct EasyStruct unsavedProjectAlertEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Unsaved Project",
    "Some changes to this project haven't been committed to disk.\nSave changes to project?",
    "Save|Don't Save|Cancel"
};

struct EasyStruct confirmRevertProjectEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Revert",
    "Are you sure you want to revert this project\nto the last saved version on disk?",
    "Revert|Don't Revert"
};

struct EasyStruct confirmRevertMapEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Revert",
    "Are you sure you want to revert map %ld \"%s\"\nto the last version saved in the project?",
    "Revert|Don't Revert"
};

struct EasyStruct confirmCreateMapEasyStruct = {
    sizeof(struct EasyStruct),
    0,
    "Confirm Create",
    "Map %ld doesn't exist yet. Create it?",
    "Create|Don't Create"
};

struct EasyStruct tilesetOutOfBoundsEasyStruct = {
  sizeof(struct EasyStruct),
  0,
  "Tileset Not In New Tileset Package",
  "This map had tileset %ld, which does not exist\nin the new package.\nThe tileset has been removed from this map.",
  "OK"
};
