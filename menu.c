#include "menu.h"

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <stdio.h>
#include <stdlib.h>

#include "framework/menu.h"
#include "framework/menubuild.h"

#include "entitiesmenu.h"
#include "mapmenu.h"
#include "musicmenu.h"
#include "projectmenu.h"

static MenuSpec mainMenuSpecData[] = {
    { "Project",  &projectMenuSpec  },
    { "Maps",     &mapMenuSpec      },
    { "Entities", &entitiesMenuSpec },
    { "Music",    &musicMenuSpec    },
    END_MENUS
};

MenuSpec *mainMenuSpec = mainMenuSpecData;

/* TODO: awkward how we need to pass in the menu... */
/*void handleMainMenuPick(struct Menu *menu, struct IntuiMessage *msg) {
    struct MenuItem *item = NULL;
    ULONG menuNumber = msg->Code;

    while(menuNumber != MENUNULL) {
        invokeMenuHandler(menuNumber);
        item = ItemAddress(menu, menuNumber);
        menuNumber = item->NextSelect;
    }
}*/
