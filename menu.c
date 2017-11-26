#include "menu.h"

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include "entitiesmenu.h"
#include "mapmenu.h"
#include "menubuild.h"
#include "musicmenu.h"
#include "projectmenu.h"

static MenuSpec menus[] = {
    { "Project",  &projectMenuSpec  },
    { "Maps",     &mapMenuSpec      },
    { "Entities", &entitiesMenuSpec },
    { "Music",    &musicMenuSpec    },
    END_MENU
};

static struct NewMenu newMenu[] = {
    { NM_TITLE, "Project", 0, 0, 0, 0 },
        { NM_ITEM, "New",                       "N", 0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Open...",                   "O", 0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Save",                      "S", 0,               0, 0 },
        { NM_ITEM, "Save As...",                "A", 0,               0, 0 },
        { NM_ITEM, "Revert",                     0,  NM_ITEMDISABLED, 0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Select Tileset Package...",  0,  0,               0, 0 },
        { NM_ITEM, NM_BARLABEL,                  0,  0,               0, 0 },
        { NM_ITEM, "Quit",                      "Q", 0,               0, 0 },
    { NM_TITLE, "Maps",   0, 0, 0, 0 },
        { NM_ITEM, "New Map",     0, 0, 0, 0 },
        { NM_ITEM, "Open Map...", 0, 0, 0, 0 },
    { NM_TITLE, "Entities",  0, 0, 0, 0 },
        { NM_ITEM, "Entity Editor...", 0, 0, 0, 0 },
    { NM_TITLE, "Music",  0, 0, 0, 0 },
        { NM_ITEM, "Song Names...", 0, 0, 0, 0 },
    { NM_END,   NULL,      0, 0, 0, 0 }
};

struct Menu *createMenu(void) {
    return CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
}
