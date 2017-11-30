#include "menu.h"

#include <libraries/gadtools.h>
#include <proto/gadtools.h>

#include <stdio.h>
#include <stdlib.h>

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
    END_MENUS
};

struct Menu *createMenu(void) {
    struct Menu *menu;

    struct NewMenu *newMenu = buildNewMenu(menus);
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
