#ifndef MENUBUILD_H
#define MENUBUILD_H

#include <exec/types.h>
#include <libraries/gadtools.h>

typedef void (*Handler)(void);

typedef struct MenuItemSpecTag {
    char *label;
    char *shortcut;
    BOOL disabled;
    Handler handler;
} MenuItemSpec;

typedef MenuItemSpec MenuSectionSpec[];

typedef struct MenuSpecTag {
    char *name;
    MenuSectionSpec *(*sections)[];
} MenuSpec;

struct NewMenu *buildNewMenu(MenuSpec*);

#define END_SECTION { NULL, NULL, FALSE, NULL }
#define END_MENU    NULL
#define END_MENUS   { NULL, NULL }

BOOL endMenuSection(MenuItemSpec*);
BOOL endMenuSpec(MenuSpec*);

#endif
