#include "menu.h"

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <intuition/intuition.h>
#include <proto/intuition.h>

#include <stdio.h>
#include <stdlib.h>

#include "framework/menubuild.h"

#include "entitiesmenu.h"
#include "mapmenu.h"
#include "musicmenu.h"
#include "projectmenu.h"

static MenuSpec mainMenuSpec[] = {
    { "Project",  &projectMenuSpec  },
    { "Maps",     &mapMenuSpec      },
    { "Entities", &entitiesMenuSpec },
    { "Music",    &musicMenuSpec    },
    END_MENUS
};

/* TODO: factor this out, move into the framework */
struct Menu *createMainMenu(void) {
    struct Menu *menu;

    struct NewMenu *newMenu = buildNewMenu(mainMenuSpec);
    if(!newMenu) {
        fprintf(stderr, "Error creating NewMenu\n");
        goto error;
    }

    menu = CreateMenus(newMenu, GTMN_FullMenu, TRUE, TAG_END);
    free(newMenu);
    return menu;
error:
    return NULL;
}

Handler getMenuItemHandler(MenuSpec *menuSpec, UWORD itemNum) {
    MenuSectionSpec **sectionSpec = &(*menuSpec->sections)[0];
    MenuItemSpec *itemSpec = &(**sectionSpec)[0];

    while(itemNum) {
        itemSpec++;
        if(endMenuSection(itemSpec)) {
            sectionSpec++;
            itemSpec = &(**sectionSpec)[0];
            /* skip past the end of section marker */
            itemNum--;
        }

        itemNum--;
    }

    return itemSpec->handler;
}

/* TODO: factor me out into the framework! */
Handler getMenuHandler(UWORD menuNum, UWORD itemNum) {
    MenuSpec *menuSpec = &mainMenuSpec[menuNum];
    return getMenuItemHandler(menuSpec, itemNum);
}

/* TODO: factor me out into the framework! */
static void invokeMenuHandler(ULONG menuNumber) {
    UWORD menuNum = MENUNUM(menuNumber);
    UWORD itemNum = ITEMNUM(menuNumber);
    
    Handler handler = getMenuHandler(menuNum, itemNum);
    handler();
}

/* TODO: awkward how we need to pass in the menu... */
void handleMainMenuPick(struct Menu *menu, struct IntuiMessage *msg) {
    struct MenuItem *item = NULL;
    ULONG menuNumber = msg->Code;

    while(menuNumber != MENUNULL) {
        invokeMenuHandler(menuNumber);
        item = ItemAddress(menu, menuNumber);
        menuNumber = item->NextSelect;
    }
}
