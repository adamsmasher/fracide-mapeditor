#ifndef MENUBUILD_H
#define MENUBUILD_H

#include <exec/types.h>

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

#define END_SECTION 0
#define END_MENU    NULL

#endif