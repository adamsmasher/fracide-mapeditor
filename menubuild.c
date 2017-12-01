#include "menubuild.h"

#include <stdio.h>
#include <stdlib.h>

static BOOL endSection(MenuItemSpec *spec) {
    return (BOOL)(spec->label == NULL);
}

static BOOL endMenuSpec(MenuSpec *spec) {
    return (BOOL)(spec->name == NULL);
}

static int sectionCount(MenuItemSpec *spec) {
    int count = 0;
    for(; !endSection(spec); spec++) {
        count++;
    }
    return count;
}

static int newMenuCount(MenuSpec *menuSpec) {
    int count = 0;

    for(; !endMenuSpec(menuSpec); menuSpec++) {
        MenuSectionSpec **i;

        /* one entry for the NM_TITLE */
        count++;

        /* count up all the sections */
        for(i = *menuSpec->sections; *i != END_MENU; i++) {
            count += sectionCount(**i);
            /* plus one, for a closing NM_BARLABEL */
            count++;
        }
        /* minus one, as the last section doesn't have an ending NM_BARLABEL */
        count--;
    }
    /* plus one for the NM_END */
    count++;

    return count;
}

static void buildMenuTitle(struct NewMenu *item, char *name) {
    struct NewMenu title = { NM_TITLE, NULL, 0, 0, 0, 0 };
    title.nm_Label = name;
    *item = title;
}

static void buildBarLabel(struct NewMenu *item) {
    struct NewMenu barLabel =  { NM_ITEM, NM_BARLABEL, 0, 0, 0, 0 };
    *item = barLabel;
}

static void buildMenuItem(struct NewMenu *item, MenuItemSpec *spec) {
    struct NewMenu newItem = { NM_ITEM, NULL, NULL, 0, 0, 0 };
    newItem.nm_Label   = spec->label;
    newItem.nm_CommKey = spec->shortcut;
    newItem.nm_Flags   = spec->disabled ? NM_ITEMDISABLED : 0;
    *item = newItem;
}

static void buildSectionAt(struct NewMenu **item, MenuSectionSpec *section) {
    MenuItemSpec *i;
    for(i = *section; !endSection(i); i++) {
        buildMenuItem(*item, i);
        (*item)++;
    }
    buildBarLabel(*item);
    (*item)++;
}

static void buildMenuEnd(struct NewMenu *item) {
    struct NewMenu end = { NM_END, NULL, NULL, 0, 0, 0 };
    *item = end;
}

static void buildMenuAt(struct NewMenu **item, MenuSpec *menuSpec) {
    MenuSectionSpec **i;

    buildMenuTitle(*item, menuSpec->name);
    (*item)++;

    for(i = *menuSpec->sections; *i != END_MENU; i++) {
        buildSectionAt(item, *i);
    }

    /* move the pointer back over the final NM_BARLABEL, to be replaced with
       either the next menu's NM_TITLE or NM_END */
    (*item)--;
}

struct NewMenu *buildNewMenu(MenuSpec *menuSpec) {
    MenuSpec *i;
    struct NewMenu *j;

    int count = newMenuCount(menuSpec);
    int size = count * sizeof(struct NewMenu);
    struct NewMenu *newMenu = malloc(size);
    if(!newMenu) {
        fprintf(stderr, "Error allocating %d bytes for new menu\n", size);
        goto error;
    }

    j = newMenu;
    for(i = menuSpec; !endMenuSpec(i); i++) {
        buildMenuAt(&j, i);
    }
    buildMenuEnd(j);

    return newMenu;
error:
    return NULL;
}
