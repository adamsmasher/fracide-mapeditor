#ifndef MENUBUILD_H
#define MENUBUILD_H

#include <exec/types.h>
#include <libraries/gadtools.h>

#include "window.h"

typedef void (*Handler)(FrameworkWindow*);

#define NO_SHORTKEY NULL

typedef enum MenuItemState_tag {
    MENU_ITEM_DISABLED,
    MENU_ITEM_ENABLED
} MenuItemState;

typedef struct MenuItemSpecTag {
    char *label;
    char *shortcut;
    MenuItemState state;
    Handler handler;
} MenuItemSpec;

typedef MenuItemSpec MenuSectionSpec[];

typedef struct MenuSpecTag {
    char *name;
    /* pointer to an array of pointers to MenuSectionSpecs */
    MenuSectionSpec *(*sections)[];
} MenuSpec;

struct NewMenu *buildNewMenu(MenuSpec*);
struct Menu *createMenu(struct NewMenu*);
BOOL layoutMenu(struct Menu*);

struct Menu *createAndLayoutMenuFromSpec(MenuSpec*);

#define END_SECTION { NULL, NULL, FALSE, NULL }
#define END_MENU    NULL
#define END_MENUS   { NULL, NULL }

BOOL endMenuSection(MenuItemSpec*);
BOOL endMenuSpec(MenuSpec*);

#endif
