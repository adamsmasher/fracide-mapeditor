#ifndef NUMBERED_LIST_H
#define NUMBERED_LIST_H

#include <exec/lists.h>
#include <exec/types.h>

typedef const char* (*GetString)(void *src, UWORD i);

struct List *newNumberedList(GetString getString, void *src, UWORD size);
void freeNumberedList(struct List*);

void numberedListSetItem(struct List*, UWORD i, const char*);

#endif
