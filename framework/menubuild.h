#ifndef MENUBUILD_H
#define MENUBUILD_H

#include <exec/types.h>
#include <libraries/gadtools.h>

typedef void (*Handler)(void);

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

#define END_SECTION { NULL, NULL, FALSE, NULL }
#define END_MENU    NULL
#define END_MENUS   { NULL, NULL }

BOOL endMenuSection(MenuItemSpec*);
BOOL endMenuSpec(MenuSpec*);

#endif
