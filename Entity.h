#ifndef ENTITY_H
#define ENTITY_H

#include <exec/types.h>

#define MAX_TAGS_PER_ENTITY 8
#define TAG_ALIAS_LENGTH 16

typedef struct Frac_tag_tag {
  UBYTE id;
  UBYTE value;
  char  alias[TAG_ALIAS_LENGTH];
} Frac_tag;

typedef struct Entity_tag {
    UBYTE entityNum;
    UBYTE row;
    UBYTE col;
    UBYTE vramSlot;
    
    int tagCnt;
    Frac_tag tags[MAX_TAGS_PER_ENTITY];
} Entity;

void entityAddNewTag(Entity*);
void entityDeleteTag(Entity*, int entityNum);

#endif
